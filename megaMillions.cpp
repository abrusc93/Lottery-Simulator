#include <iostream>
#include <random>
#include <set>
#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <unistd.h>
#include <regex>

using namespace std;

struct GameDataStruct
{
    string nextDrawDate;
    string currentJackpot;
};

// Callback function to handle libcurl response
size_t writeCallback(char* buf, size_t size, size_t nmemb, string* data) {
    if (data) {
        data->append(buf, size * nmemb);
        return size * nmemb;
    }
    return 0;
}

GameDataStruct fetchGameData(){
    cout << "Fetching game data..." << endl;

    GameDataStruct gameData;
    gameData.currentJackpot = "";
    gameData.nextDrawDate = "";

    // Initialize libcurl
    CURL* curl = curl_easy_init();
    if (!curl) {
        cerr << "Error initializing libcurl." << endl;
        return gameData;
    }

    string url = "https://www.lotteryusa.com/mega-millions/";
    string response;

    // Set libcurl options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Perform HTTP GET request
    do{
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            cerr << "Failed to fetch data: " << curl_easy_strerror(res) << endl;
            curl_easy_cleanup(curl);
            return gameData;
        }

        /* cout << "response[0]: " << response[0] << endl;
        cout << "response[1]: " << response[1] << endl;
        cout << "response[2]: " << response[2] << endl;
        cout << "response[3]: " << response[3] << endl;
        cout << "response[4]: " << response[4] << endl;
        cout << "response[5]: " << response[5] << endl;
        cout << "response[6]: " << response[6] << endl;
        cout << "response[7]: " << response[7] << endl;
        cout << "response[8]: " << response[8] << endl;
        cout << "response[9]: " << response[9] << endl; */

        if (response[0] != '<'){    // First non-null char of libcurl response should be opening HTML tag: <!DOCTYPE
                cout << "Received invalid libcurl response\nTrying again...\n" << endl;
                response = "";
                unsigned int microsecond = 1000000;
                usleep(2 * microsecond);
        }
    } while (response[0] != '<');

    // Clean up libcurl
    curl_easy_cleanup(curl);

    // Parse HTML content using libxml2
    try {
        htmlDocPtr doc = htmlReadMemory(response.c_str(), response.length(), nullptr, nullptr, HTML_PARSE_NOERROR);
        if (doc == NULL) {
            cerr << "Failed to parse HTML." << endl;
            return gameData;
        }

        xmlNodePtr rootNode = xmlDocGetRootElement(doc);
        if (rootNode == NULL) {
        cerr << "Failed to get the root element." << endl;
        xmlFreeDoc(doc);
        return gameData;
        }
        
        xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
        if (xpathCtx == NULL) {
        cerr << "Failed to create XPath context." << endl;
        xmlFreeDoc(doc);
        return gameData;
        }

        // Find Current Jackpot
        xmlChar* xpathExpr = (xmlChar*)"//dd[contains(@class, 'c-next-draw-card__prize-value')]";
        xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
        if (xpathObj == NULL) {
            cerr << "Failed to evaluate XPath expression." << endl;
            xmlXPathFreeContext(xpathCtx);
            xmlFreeDoc(doc);
            return gameData;
        }

        if (xmlXPathNodeSetIsEmpty(xpathObj->nodesetval)) {
            cout << "No matching element found while searching for current jackpot." << endl;
        } else {
            xmlNodePtr node = xpathObj->nodesetval->nodeTab[0];
            gameData.currentJackpot = (char *)xmlNodeGetContent(node);
        }

        // Find Next Draw Date
        xpathExpr = (xmlChar*)"//time[contains(@class, 'c-next-draw-card__date')]";
        xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);

        if (xpathObj == NULL) {
            cerr << "Failed to evaluate XPath expression." << endl;
            xmlXPathFreeContext(xpathCtx);
            xmlFreeDoc(doc);
            return gameData;
        }

        if (xmlXPathNodeSetIsEmpty(xpathObj->nodesetval)) {
            cout << "No matching element found while searching for next draw date." << endl;
        } else {
            xmlNodePtr node = xpathObj->nodesetval->nodeTab[0];
            gameData.nextDrawDate = (char *)xmlNodeGetContent(node);
        }

        xmlXPathFreeObject(xpathObj);
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        
    } catch (const exception& e) {
        cerr << "Error parsing HTML: " << e.what() << endl;
    }

    return gameData;
}

// Function to remove leading and trailing spaces from a string
void removeLeadingTrailingSpaces(std::string &str) {
    size_t start = 0;
    size_t end = str.length() - 1;

    // Remove leading spaces
    while (start < str.length() && std::isspace(str[start])) {
        start++;
    }

    // Remove trailing spaces
    while (end > start && std::isspace(str[end])) {
        end--;
    }

    // Erase leading and trailing spaces from the string
    str = str.substr(start, end - start + 1);

    // Remove any newlines from the string
    int n=0;
    for(int i=0;i<str.length();i++){
        if (str[i]=='\n'){
            str[i] = ' '; // replace newline with space
        }
        else{
            str[i-n]=str[i];
        }
    }
    str.resize(str.length()-n);

    // Remove any tabs from the string
    n=0;
    for(int i=0;i<str.length();i++){
        if(str[i]=='\t'){
            n++;
        }
        else{
            str[i-n]=str[i];
        }
    }
    str.resize(str.length()-n);
}

int extractIntegerWords(const string &input) {
    regex reg("(\\d+)");
    smatch match;

    // Search for integers in the input string
    regex_search(input, match, reg);

    // Convert the matched integer string to an actual integer
    int extractedInteger = 0;
    if (match.size() > 0) {
        extractedInteger = stoi(match[0]);
    }

    return extractedInteger;
}

int calculateWinnings(int matching_white, bool matchesMegaball, int jackpot, int megaplier){

    if(matching_white == 5 && matchesMegaball)
        return jackpot;

    else if(matching_white == 5)
        return 1000000 * megaplier;

    else if(matching_white == 4 && matchesMegaball)
        return 10000 * megaplier;

    else if(matching_white == 4)
        return 500 * megaplier;

    else if(matching_white == 3 && matchesMegaball)
        return 200 * megaplier;

    else if(matching_white == 3)
        return 10 * megaplier;

    else if(matching_white == 2 && matchesMegaball)
        return 10 * megaplier;

    else if(matching_white == 1 && matchesMegaball)
        return 4 * megaplier;

    else if(matchesMegaball)
        return 2 * megaplier;

    else
        return 0;
}

string formatWithCommas(int value){
    string result=to_string(value);
    for(int i=result.size()-3; i>0;i-=3)
        result.insert(i,",");
    return result;
}

int main(){
    int winning_numbers[6];
    random_device rd;
    mt19937 gen(rd());
    int play_opt;
    int num_of_plays;
    char addMegaplier;
    int megaplier;
    int matching_white;
    bool matchesMegaball;
    int total_winnings = 0;
    int random_num;
    bool done;
    set<int> chosenWinning;
    char playAgain;
   
    GameDataStruct gameData = fetchGameData();
    string jackpot_string = gameData.currentJackpot;
    string next_draw_date = gameData.nextDrawDate;

    if (jackpot_string == ""){
        cout << "Failed to fetch jackpot." << endl;
        return 0;
    }

    if (next_draw_date == ""){
        cout << "Failed to fetch next draw date." << endl;
    }

    removeLeadingTrailingSpaces(next_draw_date);

    do{
        chosenWinning.clear();
        total_winnings = 0;

        cout << "\n\n\nMEGA MILLIONS\n\nNext Draw: " << next_draw_date << " 11:00 pm\nESTIMATED JACKPOT: " << 
            jackpot_string << endl;

        //cout << "Extracted Integers from jackpot_string: " << extractIntegerWords(jackpot_string) << endl;

        const int jackpot = extractIntegerWords(jackpot_string) * 1000000;

        //cout << "Jackpot: " << formatWithCommas(jackpot) << "\n" << endl;

        //Generate random winning numbers
        for(int j=0; j<5; j++){ //Generate 5 random numbers between 1 and 70
            uniform_int_distribution<> distribution(1, 70);
            done = false;
            do{
                random_num = distribution(gen);
                if (chosenWinning.find(random_num) == chosenWinning.end()) { //Not already chosen
                    chosenWinning.insert(random_num);
                    winning_numbers[j] = random_num;
                    done = true;
                }     
            }
            while(!done);
        }
        //Generate random Megaball number between 1 and 25
        uniform_int_distribution<> distribution(1, 25);
        winning_numbers[5] = distribution(gen);
        
        while (cout << "(1) Quick pick\n(2) Pick my own numbers" << endl && ((!(cin >> play_opt))
                || (play_opt != 1 && play_opt != 2))) {
        cin.clear(); //clear bad input flag
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); //discard input
        cout << "\nInvalid input\n";
        }

        while (cout << "\nHow many plays?" << endl && !(cin >> num_of_plays)) {
        cin.clear(); //clear bad input flag
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); //discard input
        cout << "\nInvalid input\n";
        }

        do{
            cout << "\nWould you like to add Megaplier? (y/n)" << endl;
            cin >> addMegaplier;
            if(addMegaplier != 'y' && addMegaplier != 'Y' &&
                addMegaplier != 'n' && addMegaplier != 'N')
                cout << "\nInvalid input\n" << endl;
            else if(addMegaplier == 'y' || addMegaplier == 'Y'){
                uniform_int_distribution<> distribution(2, 5);
                megaplier = distribution(gen);
            }
            else if(addMegaplier == 'n'){
                megaplier = 1;
            }
        }
        while(addMegaplier != 'y' && addMegaplier != 'Y' &&
                addMegaplier != 'n' && addMegaplier != 'N');

        //Quick Pick
        if (play_opt == 1){

            cout << "\n\nYour Tickets:\n" << endl;

            for(int i=1; i<=num_of_plays; i++){
                int ticket[6];
                int ticket_winnings = 0;
                set<int> generatedNumbers;

                for(int j=0; j<5; j++){ //Generate 5 random numbers between 1 and 70
                    uniform_int_distribution<> distribution(1, 70);
                    done = false;
                    do{
                        random_num = distribution(gen);
                        if (generatedNumbers.find(random_num) == generatedNumbers.end()) { //Not already chosen
                            generatedNumbers.insert(random_num);
                            ticket[j] = random_num;
                            done = true;
                        }     
                    }
                    while(!done);
                }

                //Generate random Megaball number between 1 and 25
                uniform_int_distribution<> distribution(1, 25);
                ticket[5] = distribution(gen);

                //Print ticket and calculate winnings
                matching_white = 0;
                matchesMegaball = false;
                
                for(int num=0; num<5; num++){   //Loop through each white ball in ticket
                    for(int i=0; i<5; i++){ //Loop through each white winning number
                        if(ticket[num] == winning_numbers[i]){   //Matching white ball number
                            cout << "(" << ticket[num] << ")\t";   //Print matching white ball number in surrounding parenthases
                            matching_white += 1;
                            break;  //Match found; exit the loop
                        }
                        else    //Number in ticket does not match winning number
                            if(i==4)    //Last iteration of loop
                                cout << ticket[num] << "\t";   //Print number without parenthases
                    }
                }
                if(ticket[5] == winning_numbers[5]){    //Matching Megaball number
                    matchesMegaball = true;
                    cout << "(" << ticket[5] << ")\t"; //Print matching Megaball number in surrounding parenthases
                }
                else
                    cout << ticket[5] << "\t"; //Print Megaball number without parenthases

                ticket_winnings[ticket] = calculateWinnings(matching_white, matchesMegaball, jackpot, megaplier);

                if(ticket_winnings[ticket] != 0)
                    cout << "$" << formatWithCommas(ticket_winnings[ticket]);
                
                total_winnings += ticket_winnings[ticket];
                cout << endl;
            }
        }

        //User picks their own numbers
        else{
            int tickets[num_of_plays][6];
            int ticket_winnings[num_of_plays];
            
            for(int i=1; i<=num_of_plays; i++){
                set<int> chosenNumbers;
                int user_selected_num;
                cout << "\n\nTICKET #" << i << "\n----------\n" << endl;

                cout << "Enter 5 numbers between 1 and 70:" << endl;
                for(int j=0; j<5; j++){
                    done = false;
                    do{
                        cin >> user_selected_num;
                        if((user_selected_num < 1) || (user_selected_num > 70)) // Not in valid range
                            cout << "\nNumber must be between 1 and 70" << endl;
                        else if(!(chosenNumbers.find(user_selected_num) == chosenNumbers.end()))    // Num already chosen
                            cout << "\nNumber already chosen" << endl;
                        else    //Valid number selection
                        {
                            tickets[i-1][j] = user_selected_num;
                            chosenNumbers.insert(user_selected_num);
                            done = true;
                        }
                    }
                    while(!done);
                }

                cout << "\nPick 1 Megaball number between 1 and 25:" << endl;
                do{
                    cin >> tickets[i-1][5];
                    if((tickets[i-1][5] < 1) || (tickets[i-1][5] > 25))
                        cout << "\nNumber must be between 1 and 25" << endl;
                }
                while((tickets[i-1][5] < 1) || (tickets[i-1][5] > 25));
            }

            //Print each of user's tickets and calculate winnings
            cout << "\n\nYour Tickets:\n" << endl;
            for(int ticket=0; ticket<num_of_plays; ticket++){   //Loop through each of player's tickets
                matching_white = 0;
                matchesMegaball = false;
                
                for(int num=0; num<5; num++){   //Loop through each white ball in ticket
                    for(int i=0; i<5; i++){ //Loop through each white winning number
                        if(tickets[ticket][num] == winning_numbers[i]){   //Matching white ball number
                            cout << "(" << tickets[ticket][num] << ")\t";   //Print matching white ball number in surrounding parenthases
                            matching_white += 1;
                            break;  //Match found; exit the loop
                        }
                        else    //Number in ticket does not match winning number
                            if(i==4)    //Last iteration of loop
                                cout << tickets[ticket][num] << "\t";   //Print number without parenthases
                    }
                }
                if(tickets[ticket][5] == winning_numbers[5]){    //Matching Megaball number
                    matchesMegaball = true;
                    cout << "(" << tickets[ticket][5] << ")\t"; //Print matching Megaball number in surrounding parenthases
                }
                else
                    cout << tickets[ticket][5] << "\t"; //Print Megaball number without parenthases

                ticket_winnings[ticket] = calculateWinnings(matching_white, matchesMegaball, jackpot, megaplier);

                if(ticket_winnings[ticket] != 0)
                    cout << "$" << formatWithCommas(ticket_winnings[ticket]);
                
                total_winnings += ticket_winnings[ticket];
                cout << endl;
            }
        }

        //Print price of tickets
        if(megaplier > 1)
            cout << "\n\nPrice of Tickets: $" << formatWithCommas(3 * num_of_plays) << endl;
        else
            cout << "\n\nPrice of Tickets: $" << formatWithCommas(2 * num_of_plays) << endl;

        //Print the winning numbers
        cout << "\n\nWinning Numbers:\n" << endl;
        for(int i=0; i<6; i++){
            cout << winning_numbers[i] << "\t";
        }
        cout << endl;

        if(megaplier > 1)
            cout << "\nMEGAPLIER X" << megaplier << endl;

        if(total_winnings >= jackpot)
            cout << "\n\n\n*** CONGRATULATIONS - YOU WON THE JACKPOT! ***\n\nYour Winnings: $" << formatWithCommas(total_winnings) << endl;

        else if (total_winnings > 0)
            cout << "\n\n\n*  Your Winnings: $" << formatWithCommas(total_winnings) << "  *" << endl;
        
        else
            cout << "\n\n\nSorry, you didn't win this time." << endl;

        cout << "\n\n" << endl;

        do{
            cout << "\nPlay again? (y/n)" << endl;
            cin >> playAgain;
            if(playAgain != 'y' && playAgain != 'Y' &&
                playAgain != 'n' && playAgain != 'N')
                cout << "\nInvalid input\n" << endl;
        }
        while(playAgain != 'y' && playAgain != 'Y' &&
                playAgain != 'n' && playAgain != 'N');

    }while(playAgain == 'y' || playAgain == 'Y');

    return 0;
}