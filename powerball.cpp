#include <iostream>
#include <sstream>
#include <string>
#include <random>
#include <set>
#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <unistd.h>
#include <regex>

using namespace std;

// Callback function to handle libcurl response
size_t writeCallback(char* buf, size_t size, size_t nmemb, string* data) {
    if (data) {
        data->append(buf, size * nmemb);
        return size * nmemb;
    }
    return 0;
}

string fetchJackpot(){
    cout << "Fetching current jackpot..." << endl;

    // Initialize libcurl
    CURL* curl = curl_easy_init();
    if (!curl) {
        cerr << "Error initializing libcurl." << endl;
        return "";
    }

    string url = "https://www.texaslottery.com/export/sites/lottery/Games/Powerball/index.html";
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
            return "";
        }

        //cout << "response[6]: " << response[6] << endl;

        if (response[6] != '<'){    // First non-null char of libcurl response should be opening HTML tag: <!DOCTYPE
                cout << "Received invalid libcurl response\nTrying again...\n" << endl;
                response = "";
                unsigned int microsecond = 1000000;
                usleep(2 * microsecond);
        }
    } while (response[6] != '<');

    // Clean up libcurl
    curl_easy_cleanup(curl);

    // Parse HTML content using libxml2
    try {
        htmlDocPtr doc = htmlReadMemory(response.c_str(), response.length(), nullptr, nullptr, HTML_PARSE_NOERROR);
        if (doc == NULL) {
            cerr << "Failed to parse HTML." << endl;
            return "";
        }

        xmlNodePtr rootNode = xmlDocGetRootElement(doc);
        if (rootNode == NULL) {
        cerr << "Failed to get the root element." << endl;
        xmlFreeDoc(doc);
        return "";
        }
        
        xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
        if (xpathCtx == NULL) {
        cerr << "Failed to create XPath context." << endl;
        xmlFreeDoc(doc);
        return "";
        }

        xmlChar* xpathExpr = (xmlChar*)"//h1[contains(@class, 'sans')]";
        xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
        if (xpathObj == NULL) {
            cerr << "Failed to evaluate XPath expression." << endl;
            xmlXPathFreeContext(xpathCtx);
            xmlFreeDoc(doc);
            return "";
        }

        if (xmlXPathNodeSetIsEmpty(xpathObj->nodesetval)) {
            cout << "No matching element found." << endl;
        } else {
            xmlNodePtr node = xpathObj->nodesetval->nodeTab[0];
            return (char *)xmlNodeGetContent(node);
        }

        xmlXPathFreeObject(xpathObj);
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        
    } catch (const exception& e) {
        cerr << "Error parsing HTML: " << e.what() << endl;
    }

    return "";
}

int calculateWinnings(int matching_white, bool matchesPowerball, int jackpot, int powerPlay){

    if(matching_white == 5 && matchesPowerball)
        return jackpot;

    else if(matching_white == 5){
        if(powerPlay > 1)
            return 2 * 1000000;
        else
            return 1000000;
    }

    else if(matching_white == 4 && matchesPowerball)
        return 50000 * powerPlay;

    else if(matching_white == 4)
        return 100 * powerPlay;

    else if(matching_white == 3 && matchesPowerball)
        return 100 * powerPlay;

    else if(matching_white == 3)
        return 7 * powerPlay;

    else if(matching_white == 2 && matchesPowerball)
        return 7 * powerPlay;

    else if(matching_white == 1 && matchesPowerball)
        return 4 * powerPlay;

    else if(matchesPowerball)
        return 4 * powerPlay;

    else
        return 0;
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
    string jackpot_string = "";
    int play_opt;
    int num_of_plays;
    char addPowerPlay;
    int powerPlay;
    int matching_white;
    bool matchesPowerball;
    int total_winnings = 0;
    int random_num;
    bool done;
    set<int> chosenWinning;

    //Generate random winning numbers

     for(int j=0; j<5; j++){ //Generate 5 random numbers between 1 and 69
        uniform_int_distribution<> distribution(1, 69);
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
    //Generate random Powerball number between 1 and 26
    uniform_int_distribution<> distribution(1, 26);
    winning_numbers[5] = distribution(gen);
 
    jackpot_string = fetchJackpot();

    if (jackpot_string != "")
        cout << "\n\n\nP O W E R BALL\nEstimated Jackpot: " << jackpot_string << "\n\n";
    else{
        cout << "Failed to fetch jackpot." << endl;
        return 0;
    }

    //cout << "Extracted Integers from jackpot_string: " << extractIntegerWords(jackpot_string) << endl;

    const int jackpot = extractIntegerWords(jackpot_string) * 1000000;

    //cout << "Jackpot: " << formatWithCommas(jackpot) << "\n" << endl;

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
        cout << "\nWould you like to add Power Play? (y/n)" << endl;
        cin >> addPowerPlay;
        if(addPowerPlay != 'y' && addPowerPlay != 'Y' &&
            addPowerPlay != 'n' && addPowerPlay != 'N')
            cout << "\nInvalid input\n" << endl;
        else if(addPowerPlay == 'y' || addPowerPlay == 'Y'){
            do{
                //Power Play can be 2X, 3X, 4X, 5X, or 10X
                uniform_int_distribution<> distribution(2, 10);
                powerPlay = distribution(gen);
            }
            while (powerPlay == 6 || powerPlay == 7 || powerPlay == 8 || powerPlay == 9);
        }
        else if(addPowerPlay == 'n'){
            powerPlay = 1;
        }
    }
    while(addPowerPlay != 'y' && addPowerPlay != 'Y' &&
            addPowerPlay != 'n' && addPowerPlay != 'N');

    //Quick Pick
    if (play_opt == 1){

        cout << "\n\nYour Tickets:\n" << endl;

        for(int i=1; i<=num_of_plays; i++){
            int ticket[6];
            int ticket_winnings = 0;
            set<int> generatedNumbers;

            for(int j=0; j<5; j++){ //Generate 5 random numbers between 1 and 69
                uniform_int_distribution<> distribution(1, 69);
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

            //Generate random Powerball number between 1 and 26
            uniform_int_distribution<> distribution(1, 26);
            ticket[5] = distribution(gen);

            //Print ticket and calculate winnings
            matching_white = 0;
            matchesPowerball = false;
            
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
            if(ticket[5] == winning_numbers[5]){    //Matching Powerball number
                matchesPowerball = true;
                cout << "(" << ticket[5] << ")\t"; //Print matching Powerball number in surrounding parenthases
            }
            else
                cout << ticket[5] << "\t"; //Print powerball number without parenthases

            ticket_winnings[ticket] = calculateWinnings(matching_white, matchesPowerball, jackpot, powerPlay);

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

            cout << "Enter 5 numbers between 1 and 69:" << endl;
            for(int j=0; j<5; j++){
                done = false;
                do{
                    cin >> user_selected_num;
                    if((user_selected_num < 1) || (user_selected_num > 69)) // Not in valid range
                        cout << "\nNumber must be between 1 and 69" << endl;
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

            cout << "\nPick 1 Powerball number between 1 and 26:" << endl;
            do{
                cin >> tickets[i-1][5];
                if((tickets[i-1][5] < 1) || (tickets[i-1][5] > 26))
                    cout << "\nNumber must be between 1 and 26" << endl;
            }
            while((tickets[i-1][5] < 1) || (tickets[i-1][5] > 26));
        }

        //Print each of user's tickets and calculate winnings
        cout << "\n\nYour Tickets:\n" << endl;
        for(int ticket=0; ticket<num_of_plays; ticket++){   //Loop through each of player's tickets
            matching_white = 0;
            matchesPowerball = false;
            
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
            if(tickets[ticket][5] == winning_numbers[5]){    //Matching Powerball number
                matchesPowerball = true;
                cout << "(" << tickets[ticket][5] << ")\t"; //Print matching Powerball number in surrounding parenthases
            }
            else
                cout << tickets[ticket][5] << "\t"; //Print powerball number without parenthases

            ticket_winnings[ticket] = calculateWinnings(matching_white, matchesPowerball, jackpot, powerPlay);

            if(ticket_winnings[ticket] != 0)
                cout << "$" << formatWithCommas(ticket_winnings[ticket]);
            
            total_winnings += ticket_winnings[ticket];
            cout << endl;
        }
    }

    //Print price of tickets
    if(powerPlay > 1)
        cout << "\n\nPrice of Tickets: $" << formatWithCommas(3 * num_of_plays) << endl;
    else
        cout << "\n\nPrice of Tickets: $" << formatWithCommas(2 * num_of_plays) << endl;

    //Print the winning numbers
    cout << "\n\nWinning Numbers:\n" << endl;
    for(int i=0; i<6; i++){
        cout << winning_numbers[i] << "\t";
    }
    cout << endl;

    if(powerPlay > 1)
        cout << "\nPOWERPLAY X" << powerPlay << endl;

    if(total_winnings >= jackpot)
        cout << "\n\n\n*** CONGRATULATIONS - YOU WON THE JACKPOT! ***\n\nYour Winnings: $" << formatWithCommas(total_winnings) << endl;

    else if (total_winnings > 0)
        cout << "\n\n\n*  Your Winnings: $" << formatWithCommas(total_winnings) << "  *" << endl;
    
    else
        cout << "\n\n\nSorry, you didn't win this time." << endl;

    cout << "\n\n\n" << endl;

    return 0;
}
