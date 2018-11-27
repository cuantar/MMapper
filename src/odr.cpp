// This generated file exists to help detect ODR violations.
// It will catch duplicate class names in header files,
// but it will NOT catch duplicate names in source files.

#include "client/ClientTelnet.h"
#include "client/clientwidget.h"
#include "client/displaywidget.h"
#include "client/inputwidget.h"
#include "client/stackedinputwidget.h"
#include "clock/mumeclock.h"
#include "clock/mumeclockwidget.h"
#include "clock/mumemoment.h"
#include "configuration/configuration.h"
#include "display/ConnectionLineBuilder.h"
#include "display/connectionselection.h"
#include "display/Filenames.h"
#include "display/FontFormatFlags.h"
#include "display/MapCanvasData.h"
#include "display/mapcanvas.h"
#include "display/MapCanvasRoomDrawer.h"
#include "display/mapwindow.h"
#include "display/OpenGL.h"
#include "display/prespammedpath.h"
#include "display/RoadIndex.h"
#include "expandoracommon/AbstractRoomFactory.h"
#include "expandoracommon/coordinate.h"
#include "expandoracommon/exit.h"
#include "expandoracommon/frustum.h"
#include "expandoracommon/listcycler.h"
#include "expandoracommon/parseevent.h"
#include "expandoracommon/property.h"
#include "expandoracommon/RoomAdmin.h"
#include "expandoracommon/room.h"
#include "expandoracommon/RoomRecipient.h"
#include "global/bits.h"
#include "global/CharBuffer.h"
#include "global/Color.h"
#include "global/DirectionType.h"
#include "global/EnumIndexedArray.h"
#include "global/enums.h"
#include "global/Flags.h"
#include "global/io.h"
#include "global/NullPointerException.h"
#include "global/RAII.h"
#include "global/range.h"
#include "global/roomid.h"
#include "global/StringView.h"
#include "global/utils.h"
#include "mainwindow/aboutdialog.h"
#include "mainwindow/findroomsdlg.h"
#include "mainwindow/infomarkseditdlg.h"
#include "mainwindow/mainwindow.h"
#include "mainwindow/roomeditattrdlg.h"
#include "mainwindow/welcomewidget.h"
#include "mapdata/customaction.h"
#include "mapdata/DoorFlags.h"
#include "mapdata/drawstream.h"
#include "mapdata/enums.h"
#include "mapdata/ExitDirection.h"
#include "mapdata/ExitFieldVariant.h"
#include "mapdata/ExitFlags.h"
#include "mapdata/infomark.h"
#include "mapdata/mapdata.h"
#include "mapdata/mmapper2exit.h"
#include "mapdata/mmapper2room.h"
#include "mapdata/roomfactory.h"
#include "mapdata/roomfilter.h"
#include "mapdata/roomselection.h"
#include "mapdata/shortestpath.h"
#include "mapfrontend/AbstractRoomVisitor.h"
#include "mapfrontend/mapaction.h"
#include "mapfrontend/mapfrontend.h"
#include "mapfrontend/map.h"
#include "mapfrontend/ParseTree.h"
#include "mapfrontend/roomcollection.h"
#include "mapfrontend/roomlocker.h"
#include "mapstorage/abstractmapstorage.h"
#include "mapstorage/basemapsavefilter.h"
#include "mapstorage/filesaver.h"
#include "mapstorage/jsonmapstorage.h"
#include "mapstorage/mapstorage.h"
#include "mapstorage/progresscounter.h"
#include "mapstorage/roomsaver.h"
#include "mapstorage/StorageUtils.h"
#include "mpi/mpifilter.h"
#include "mpi/remoteedit.h"
#include "mpi/remoteeditprocess.h"
#include "mpi/remoteeditsession.h"
#include "mpi/remoteeditwidget.h"
#include "pandoragroup/CGroupChar.h"
#include "pandoragroup/CGroupClient.h"
#include "pandoragroup/CGroupCommunicator.h"
#include "pandoragroup/CGroup.h"
#include "pandoragroup/CGroupServer.h"
#include "pandoragroup/groupaction.h"
#include "pandoragroup/groupauthority.h"
#include "pandoragroup/groupselection.h"
#include "pandoragroup/groupwidget.h"
#include "pandoragroup/mmapper2group.h"
#include "parser/Abbrev.h"
#include "parser/AbstractParser-Commands.h"
#include "parser/abstractparser.h"
#include "parser/AbstractParser-Utils.h"
#include "parser/CommandId.h"
#include "parser/ConnectedRoomFlags.h"
#include "parser/DoorAction.h"
#include "parser/ExitsFlags.h"
#include "parser/mumexmlparser.h"
#include "parser/parserutils.h"
#include "parser/patterns.h"
#include "parser/PromptFlags.h"
#include "pathmachine/approved.h"
#include "pathmachine/crossover.h"
#include "pathmachine/experimenting.h"
#include "pathmachine/forced.h"
#include "pathmachine/mmapper2pathmachine.h"
#include "pathmachine/onebyone.h"
#include "pathmachine/path.h"
#include "pathmachine/pathmachine.h"
#include "pathmachine/pathparameters.h"
#include "pathmachine/roomsignalhandler.h"
#include "pathmachine/syncing.h"
#include "preferences/ansicombo.h"
#include "preferences/clientpage.h"
#include "preferences/configdialog.h"
#include "preferences/generalpage.h"
#include "preferences/graphicspage.h"
#include "preferences/groupmanagerpage.h"
#include "preferences/mumeprotocolpage.h"
#include "preferences/parserpage.h"
#include "preferences/pathmachinepage.h"
#include "proxy/AbstractTelnet.h"
#include "proxy/connectionlistener.h"
#include "proxy/MudTelnet.h"
#include "proxy/mumesocket.h"
#include "proxy/proxy.h"
#include "proxy/telnetfilter.h"
#include "proxy/TextCodec.h"
#include "proxy/UserTelnet.h"
