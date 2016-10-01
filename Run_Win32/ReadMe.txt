_______          __                       __   .__
\      \   _____/  |___  _  _____________|  | _|__| ____    ____
/   |   \_/ __ \   __\ \/ \/ /  _ \_  __ \  |/ /  |/    \  / ___\
/    |    \  ___/|  |  \     (  <_> )  | \/    <|  |   |  \/ /_/  >
\____|__  /\___  >__|   \/\_/ \____/|__|  |__|_ \__|___|  /\___  /
       \/     \/                              \/       \//_____/
 _________
/   _____/_  _  _______     ____
\_____  \\ \/ \/ /\__  \   / ___\
/        \\     /  / __ \_/ /_/  >
/_______  / \/\_/  (____  /\___  /
       \/              \//_____/

How to test:
  When the game starts, we automatically run nsstart to start the netsession.
  Press H to host on one, J on the other. You might be able to get multi-join to work, but there's a known issue with my port cycling that I haven't been able to fix yet.

  Use WASD to move your character and Spacebar to swing your sword.
    You're left-handed, so keep in mind where your sword will come out from.
    Players play a little hurt sound when they get hurt ;w;

  The appropriate states are in the game, and players are able to die, but it's very boring, and they don't disconnect, instead just watch.

Known Issues:
  - The Console is a piece of shit as always
  - I had a windows update last night that afterwards made a bunch of problems with the player speed present themselves. I fixed them by enabling vsync again. HAVE VSYNC ON PLEASE XD
  - Controls feel kinda garbage, as they should for this milestone
  - Players just monitor the battlefield, there is no "win" state or anything yet.
	- If your player doesn't spawn in when joining, it dropped the packet for joining. close window and rejoin.
