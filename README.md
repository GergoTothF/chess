# Js-Gui2

This installer project is created for demonstrating how easily can a computer play chess nowdays. 
It's made for myself and lots of ideas were taken from CPW, chess programming wiki. 
This is using bitmap inside, but with macro move generation can be switched to normal (working with for cycles). 
But evaluation is made with bitmap (it can be also switched off with macro to the simpiest evaluation).
For bitmap part 64 bit compiler is needed.
The main power is in this chess program this is running through web (using html + javascript). 
Webassambly is used is used inside javascript (compiled by Emscripten). 
Working instance is hosted by Firebase and available here: https://winchessers.firebaseapp.com/

## Project structure: 
In the base map i use emscripten (emcc) under windows. Version of emscripten is 1.38.31, clang version 6.0.1, 
and as fas as i rember Under Windows only Mingw was supported. 
In js-gui folder there is a Visual Studio 2017 project containg a main fuction which is computing a position given by fen. 
In js-gui-test there is a Visual Studio 2017 project which contains a google test checking evaluation. 
All project use the same source code and are planned to change to CMakeLists projects. 

## How to compile emscripten:
1. In my emsdk folder i should run: emsdk_env.bat under Windows.
2. Then i should compile with emcc from base folder. Commands can be placed in emcc.txt.
3. Locally i can run javascript + html from base folder with command: http-server under Windows. 
   Be watch, since if there was a public folder in root directory then http-server tries to search index.html in it. 

## Very imortant improvements and known bugs (these haven't been implemented yet due to lack of time):
- Animations is too slow due to test phase (1 hour to set correctly). 
- Stepping during animation can cause GUI crash. Some part of the GUI should be disabled during animation, or during computer thinking (2 days to solve)
- The jumping on step link is failed in case of castling (enpassant?, promotion?) -> 1 day to solve
- lack of manual testing of whole game play (1 week)
- lack of several unit and component testing (2 weeks)
- code refactor since sometime i minimalized time to get a new feature (lots of unused macros should be removed) -> 2 weeks effort
- lack of checked state messaging (1 day effort)

## Imporvements, TO-DO list:
- There is a plus improvement in it: 2 threads can be used for computing. Very hard to implement such solution, 
  since the data sharing between wasm instances is (almost?) impossible.
  Emscripten threads (pthread) is only allowed under Chrome due to a security vulnerability.
- Introduce CMake (2 days effort)
- the captured pieces can be shown somewhere in the GUI (4 days effort)
- board numbering, annotations should be outside the board (2 days)
- steps are shown in PGN format (e2e4 -> e4) -> 3 days effort
- better usage of message center: for example when the step isn't valid then message should be come about it (2 days effort)
- check the step linking GUI no it can be jumped to a previos state, but continueing the game from previous state is buggy.
  So i should disable it or solve this issue. In case jump back to the last step the game can be continued correctly. -> 3 days effort
- Maybe timer can be shown for players.
- Computer can play easily with only black color. -> 4 days effort
- Better opening implementations (2 weeks)   
- improved algorithm for example more pruning (needs at least 2 weeks with testing)
- randomized computer step (a little)
  
## Strength of the program:
It should be tested well, but no i feel it works somewhere between 1350 - 15500.
When it played against an 1150 strenth simulation it could easily win. Here is the party:

## Some experience earned during the project:
- The today's (not AI based) chess program power is on pruning, moving order, saved positions amd searching cut off.
  Not the most important thing is the machine CPU core numbers and the CPU strength.
  Without any cut off approximatelly an I5 9th gen CPU can only ananlyse 100 - 200 million state for example under 2 minutes.
  Which is only enough for thinking forward 6 ply (3 moving pair). Not enough. With cuting off algorithm it could be 6 move pairs (12 ploes) or more.
- When analyseing bugs the Google Sanitizer can give me very much help.
- I earned Emscripten, Javascript experience.
- When playing a chess i can ananlyse more eaily the positions with the knowledge of evaluation weights from the program.

## Licensing:
I used basically to programs: chessboard js and CPW engine. 
Chessboard js: https://github.com/oakmac/chessboardjs 
CPW: https://github.com/nescitus/cpw-engine

From both of the programs i have taken several ideas.
I copied here: their licences. These are basically free software.
I have never copy-paste lines from them but i copied lines from them by hand manually.
For example evaluation weight are taken from CPW, i didn't modify them.

My software is also free: [MIT License](LICENSE.md)

## Project places: 
Avalable on Github and BitBucket:
https://bitbucket.org/gergo444888/js-gui2/src/master/
https://github.com/GergoTothF/chess