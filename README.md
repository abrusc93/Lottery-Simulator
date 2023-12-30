# Lottery Simulator
 Powerball, MegaMillions, and Jersey Cash 5 lottery game simulator coded in C++

These simulated lottery games follow the official game rules to determine winning numbers and prizes and use web scraping to get the actual current jackpot.
Playing is a fun way to test your luck without the risks associated with gambling using real money. The programs use libcurl and libxml2 libraries which must be
installed on your system to compile the source files.

To compile the cpp source files, run the following commands in your terminal or command prompt and run the executable file:

g++ -std=c++11 megaMillions.cpp -o MegaMillions -lcurl `pkg-config libxml-2.0 --cflags --libs`

g++ -std=c++11 powerball.cpp -o Powerball -lcurl `pkg-config libxml-2.0 --cflags --libs`

g++ -std=c++11 jerseyCash5.cpp -o JerseyCash5 -lcurl `pkg-config libxml-2.0 --cflags --libs`

 
MegaMillions Game Rules and Prizes:
https://www.njlottery.com/en-us/drawgames/megamillions.html#tab-howToPlay
https://www.njlottery.com/en-us/drawgames/megamillions.html#tab-oddsAndPrizes

Powerball Game Rules and Prizes:
https://www.njlottery.com/en-us/drawgames/powerball.html#tab-howToPlay
https://www.njlottery.com/en-us/drawgames/powerball.html#tab-oddsAndPrizes

Jersey Cash 5 Game Rules and Prizes:
https://www.njlottery.com/en-us/drawgames/jerseycash.html#tab-howToPlay
https://www.njlottery.com/en-us/drawgames/jerseycash.html#tab-oddsAndPrizes
