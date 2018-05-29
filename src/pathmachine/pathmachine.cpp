/************************************************************************
**
** Authors:   Ulf Hermann <ulfonk_mennhar@gmx.de> (Alve),
**            Marek Krejza <krejza@gmail.com> (Caligor),
**            Nils Schimmelmann <nschimme@gmail.com> (Jahara)
**
** This file is part of the MMapper project.
** Maintained by Nils Schimmelmann <nschimme@gmail.com>
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the:
** Free Software Foundation, Inc.
** 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
************************************************************************/

#include "pathmachine.h"
#include "abstractroomfactory.h"
#include "approved.h"
#include "crossover.h"
#include "customaction.h"
#include "mmapper2event.h"
#include "mmapper2room.h"
#include "onebyone.h"
#include "parseevent.h"
#include "path.h"
#include "syncing.h"

#include <stack>

PathMachine::PathMachine(AbstractRoomFactory *in_factory, bool threaded) :
    Component(threaded),
    factory(in_factory),
    signaler(this),
    pathRoot(0, 0, 0),
    mostLikelyRoom(0, 0, 0),
    lastEvent(Mmapper2Event::createEvent(CID_UNKNOWN, nullptr, nullptr, nullptr, 0, 0, 0)),
    state(SYNCING),
    paths(new std::list<Path *>)
{}

void PathMachine::setCurrentRoom(Approved *app)
{
    releaseAllPaths();
    const Room *perhaps = app->oneMatch();
    if (perhaps != nullptr) {
        mostLikelyRoom = *perhaps;
        emit playerMoved(mostLikelyRoom.getPosition());
        state = APPROVED;
    }
}

void PathMachine::setCurrentRoom(const Coordinate &pos)
{
    if (lastEvent == nullptr) {
        abort();
    }
    Approved app(factory, *lastEvent, 100);
    emit lookingForRooms(&app, pos);
    setCurrentRoom(&app);
}

void PathMachine::setCurrentRoom(uint id)
{
    if (lastEvent == nullptr) {
        abort();
    }
    Approved app(factory, *lastEvent, 100);
    emit lookingForRooms(&app, id);
    setCurrentRoom(&app);
}


void PathMachine::init()
{
    connect(&signaler, &RoomSignalHandler::scheduleAction, this, &PathMachine::scheduleAction);
}

void PathMachine::releaseAllPaths()
{
    for (auto &path : *paths) {
        path->deny();
    }
    paths->clear();

    state = SYNCING;
}

void PathMachine::retry()
{

    switch (state) {
    case APPROVED:
        state = SYNCING;
        break;
    case EXPERIMENTING:
        releaseAllPaths();
        break;
    }
    if (lastEvent != nullptr) {
        event(lastEvent);
    }
}

void PathMachine::event(ParseEvent *ev)
{
    if (ev != lastEvent) {
        delete lastEvent;
        lastEvent = ev;
    }
    switch (state) {
    case APPROVED:
        approved(*ev);
        break;
    case EXPERIMENTING:
        experimenting(*ev);
        break;
    case SYNCING:
        syncing(*ev);
        break;
    }
}

void PathMachine::tryExits(const Room *room, RoomRecipient *recipient, ParseEvent &event, bool out)
{
    uint move = event.getMoveType();
    if (move < static_cast<uint>(room->getExitsList().size())) {
        const Exit &possible = room->exit(move);
        tryExit(possible, recipient, out);
    } else {
        emit lookingForRooms(recipient, room->getId());
        if (move >= factory->numKnownDirs()) {
            const ExitsList &eList = room->getExitsList();
            for (const auto &possible : eList) {
                tryExit(possible, recipient, out);
            }
        }
    }
}

void PathMachine::tryExit(const Exit &possible, RoomRecipient *recipient, bool out)
{
    for (auto idx : possible.getRange(out)) {
        emit lookingForRooms(recipient, idx);
    }
}


void PathMachine::tryCoordinate(const Room *room, RoomRecipient *recipient, ParseEvent &event)
{
    uint moveCode = event.getMoveType();
    uint size = factory->numKnownDirs();
    if (size > moveCode) {
        Coordinate c = room->getPosition() + factory->exitDir(moveCode);
        emit lookingForRooms(recipient, c);
    } else {
        Coordinate roomPos = room->getPosition();
        for (uint i = 0; i < size; ++i) {
            emit lookingForRooms(recipient, roomPos + factory->exitDir(i));
        }
    }
}

void PathMachine::approved(ParseEvent &event)
{
    Approved appr(factory, event, params.matchingTolerance);
    const Room *perhaps = nullptr;

    tryExits(&mostLikelyRoom, &appr, event, true);

    perhaps = appr.oneMatch();

    if (perhaps == nullptr) {
        // try to match by reverse exit
        appr.reset();
        tryExits(&mostLikelyRoom, &appr, event, false);
        perhaps = appr.oneMatch();
        if (perhaps == nullptr) {
            // try to match by coordinate
            appr.reset();
            tryCoordinate(&mostLikelyRoom, &appr, event);
            perhaps = appr.oneMatch();
            if (perhaps == nullptr) {
                // try to match by coordinate one step below expected
                const Coordinate &eDir = factory->exitDir(event.getMoveType());
                if (eDir.z == 0) {
                    appr.reset();
                    Coordinate c = mostLikelyRoom.getPosition() + eDir;
                    c.z--;
                    emit lookingForRooms(&appr, c);
                    perhaps = appr.oneMatch();

                    if (perhaps == nullptr) {
                        // try to match by coordinate one step above expected
                        appr.reset();
                        c.z += 2;
                        emit lookingForRooms(&appr, c);
                        perhaps = appr.oneMatch();
                    }
                }
            }
        }
    }
    if (perhaps != nullptr) {
        // Update the exit from the previous room to the current room
        uint move = event.getMoveType();
        if (static_cast<uint>(mostLikelyRoom.getExitsList().size()) > move) {
            emit scheduleAction(new AddExit(mostLikelyRoom.getId(), perhaps->getId(), move));
        }

        // Update most likely room with player's current location
        mostLikelyRoom = *perhaps;

        // Update rooms behind exits now that we are certain about our current location
        ConnectedRoomFlagsType bFlags = Mmapper2Event::getConnectedRoomFlags(event);
        if ((bFlags & CONNECTED_ROOM_FLAGS_VALID) != 0) {
            for (uint dir = 0; dir < 6; ++dir) {
                const Exit &e = mostLikelyRoom.exit(dir);
                if (!e.outIsUnique()) {
                    continue;
                }
                uint connectedRoomId = e.outFirst();
                ConnectedRoomFlagsType bThisRoom = bFlags >> (dir * 2);
                if ((bThisRoom & DIRECT_SUN_ROOM) != 0) {
                    emit scheduleAction(new SingleRoomAction(new UpdateRoomField(RST_SUNDEATH, R_SUNDEATHTYPE),
                                                             connectedRoomId));
                } else if ((bThisRoom & INDIRECT_SUN_ROOM) != 0) {
                    emit scheduleAction(new SingleRoomAction(new UpdateRoomField(RST_NOSUNDEATH, R_SUNDEATHTYPE),
                                                             connectedRoomId));
                }
            }
        }

        // Send updates
        emit playerMoved(mostLikelyRoom.getPosition());
        // GroupManager
        emit setCharPosition(mostLikelyRoom.getId());
    } else {
        // couldn't match, give up
        state = EXPERIMENTING;
        pathRoot = mostLikelyRoom;
        auto *root = new Path(&pathRoot, nullptr, nullptr, &signaler);
        paths->push_front(root);
        experimenting(event);
    }
}


void PathMachine::syncing(ParseEvent &event)
{
    {
        Syncing sync(params, paths, &signaler);
        if (event.getNumSkipped() <= params.maxSkipped) {
            emit lookingForRooms(&sync, event);
        }
        paths = sync.evaluate();
    }
    evaluatePaths();
}


void PathMachine::experimenting(ParseEvent &event)
{
    Experimenting *exp = nullptr;
    uint moveCode = event.getMoveType();
    Coordinate move = factory->exitDir(moveCode);
    // only create rooms if no properties are skipped and
    // the move coordinate is not 0,0,0
    if (event.getNumSkipped() == 0
            && (moveCode < static_cast<uint>(mostLikelyRoom.getExitsList().size()))) {
        exp = new Crossover(paths, moveCode, params, factory);
        std::set<const Room *> pathEnds;
        for (auto &path : *paths) {
            const Room *working = path->getRoom();
            if (pathEnds.find(working) == pathEnds.end()) {
                emit createRoom(event, working->getPosition() + move);
                pathEnds.insert(working);
            }
        }
        emit lookingForRooms(exp, event);
    } else {
        auto *oneByOne = new OneByOne(factory, &event, params, &signaler);
        exp = oneByOne;
        for (auto &path : *paths) {
            const Room *working = path->getRoom();
            oneByOne->addPath(path);
            tryExits(working, exp, event, true);
            tryExits(working, exp, event, false);
            tryCoordinate(working, exp, event);
        }
    }

    paths = exp->evaluate();
    delete exp;
    evaluatePaths();
}

void PathMachine::evaluatePaths()
{
    if (paths->empty()) {
        state = SYNCING;
    } else {
        mostLikelyRoom = *(paths->front()->getRoom());
        if (++paths->begin() == paths->end()) {
            state = APPROVED;
            paths->front()->approve();
            paths->pop_front();
        } else {
            state = EXPERIMENTING;
        }
        emit playerMoved(mostLikelyRoom.getPosition());
    }
}





