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
  Run nethost <username> on one, netjoin <username> <ip> on the other
  I conformed to your standards, so things should be pretty straightforward


  If you'd like to test inorder vs outoforder reliable traffic, I created the following two commands to toggle debugging the two:
  testinorder
    Sends reliable, inorder "heartbeats". You can follow along with the counter in the console and watch them go up.
    Note that I'm sending these to the "default" local host with the default port, so whichever dude joins first (4334) currently gets them.
  testoutorder
    Sends reliable, out of order "heartbeats". You can follow the counter and watch them get in the wrong order.
    Same deal as above with the port thing, since I'm just defaulting to 4334.

Known Issues:
  - The Console is a piece of shit as always
  - Note that I'm currently sending packets to myself to enable you to test without a partner and for debug reasons, this obviously isn't what I should do in real life
