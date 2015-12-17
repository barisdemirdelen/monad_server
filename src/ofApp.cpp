#include "ofApp.h"

/*
 texture patterns can be randomized
 
 colors for users:
 161,24,87 - purple
 255,153,0 - orange
 50,153,187 - blue
 
 */


//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetVerticalSync(true);
    //    ofBackground(255);
    
    //set up network
    server.setup(10002);
    server.setMessageDelimiter("varianet");
    
    // set up values of objects
    disc.setup();

    
//    sound.setup(&disc);
    
    // set up game costs
    costRadius = -1;
    costDensity = -1;
    costRotation = -1;
    costTexture = -2;
    costMute = -2;
    costMove = -2;
    reward = 4;
}
//--------------------------------------------------------------
void ofApp::exit(){
    
    for(int i = 0; i < players.size(); i++){
        delete players[i];
    }
    
}
//--------------------------------------------------------------

void ofApp::update(){
    
    disc.update();
    
    for ( int i = 0; i < server.getLastID(); i++ ) {
        if(server.isClientConnected(i)) {
            string str = server.receive(i);
            if(str.length()>0){
                received = ofSplitString(str, "//");
                title = received[0];
                
                ////////////send to all other clients////////
                for (int j = 0; j < server.getLastID(); j++) {
                    if(j != i && title != "hello") server.send(j, str);
                }
                
                /////////////// initialize /////////////////
                if (title == "hello") {
                    
                    Player* _player = new Player();
                    string thisIP = server.getClientIP(i);
                    _player->setIP(thisIP);
                    bool rejoin = false;
                    for(int j = 0; j < players.size(); j++){
                        if(_player->getIP() == players[j]->getIP()) {
                            _player = players[j];   // if the IP matches do not create new player
                            rejoin = true;
                            break;
                        }
                    }
                    if(rejoin == false) {           // make new player
                        players.push_back(_player);
                        _player->setLife(100);
                        _player->setDiscIndex(-1);
                        _player->setColorByIndex(players.size());
                        _player->setNick(received[1]);
                    }
                    _player->setConnection(true);
                    
                    //send game costs
                    string costs = "costs//";
                    costs += "costRadius: " + ofToString(costRadius) + "//";
                    costs += "costDensity: " + ofToString(costDensity) + "//";
                    costs += "costRotation: " + ofToString(costRotation) + "//";
                    costs += "costTexture: " + ofToString(costTexture) + "//";
                    costs += "costMute: " + ofToString(costMute) + "//";
                    costs += "costMove: " + ofToString(costMove) + "//";
                    costs += "reward: " + ofToString(reward) + "//";
                    server.send(i, costs);
                    
                    
                    string playerInfo; // send player status
                    playerInfo += "playerInfo//";
                    playerInfo += "IP: " + ofToString(_player->getIP()) + "//";
                    playerInfo += "color: " + ofToString(_player->getColor()) + "//";
                    playerInfo += "life: " + ofToString(_player->getLife()) + "//";
                    playerInfo += "index: " + ofToString(_player->getDiscIndex()) + "//";
                    if(rejoin == true) playerInfo += "nick: " + _player->getNick() + "//";
                    server.send(i, playerInfo);
                    
                    if(players.size() > 1){ //if there are other players, also send their info
                        for(int j = 0; j < players.size(); j++){
                            if( players[j] != _player && players[j]->isConnected()){
                                string otherPlayers = "otherPlayers//";
                                otherPlayers += "IP: " + ofToString(players[j]->getIP()) + "//";
                                otherPlayers += "color: " + ofToString(players[j]->getColor()) + "//";
                                otherPlayers += "life: " + ofToString(players[j]->getLife()) + "//";
                                otherPlayers += "index: " + ofToString(players[j]->getDiscIndex()) + "//";
                                otherPlayers += "nick: " + players[j]->getNick() + "//";
                                server.send(i, otherPlayers);
                            }
                        }
                    }
                    
                    string newPlayerInfo;   //send player information to all other clients
                    newPlayerInfo += "otherPlayers//";
                    newPlayerInfo += "IP: " + ofToString(_player->getIP()) + "//";
                    newPlayerInfo += "color: " + ofToString(_player->getColor()) + "//";
                    newPlayerInfo += "life: " + ofToString(_player->getLife()) + "//";
                    newPlayerInfo += "index: " + ofToString(_player->getDiscIndex()) + "//";
                    newPlayerInfo += "nick: " + _player->getNick() + "//";
                    for (int j = 0; j < server.getLastID(); j++) {
                        if(j != i) server.send(j, newPlayerInfo);
                    }
                    
                    string state;   //prepare to send the current state of the server
                    state += "state//";
                    state += "discIndex: " + ofToString(disc.getDiscIndex()) + "//";
                    for(int j = 0; j < disc.getDiscIndex(); j++){
                        state += "radius"+ofToString(j) + ": " + ofToString(disc.getRadius(j)) + "//";
                        state += "rotation"+ofToString(j) + ": " + ofToString(disc.getRotation(j)) + "//";
                        state += "rotationSpeed"+ofToString(j) + ": " + ofToString(disc.getNetRotationSpeed(j)) + "//";
                        state += "density"+ofToString(j) + ": " + ofToString(disc.getDensity(j)) + "//";
                        state += "texture"+ofToString(j) + ": " + ofToString(disc.getTexture(j)) + "//";
                        state += "zPosition"+ofToString(j) + ": " + ofToString(disc.getPosition(j)) + "//";
                        state += "seed"+ofToString(j) + ": " + ofToString(disc.getSeed(j)) + "//";
                        state += "counter"+ofToString(j) + ": "+ ofToString(disc.getCounter(j)) + "//";
                        state += "mute"+ofToString(j) + ": " + ofToString(disc.isMute(j)) + "//";
                        state += "move"+ofToString(j) + ": " + ofToString(disc.isMoving(j)) + "//";
                    }
                    server.send(i, state);  //send current state to new client
                    
                    
                    cout<< "initialize " + ofToString(server.getClientIP(i)) <<endl;
                    
                }
                /////////////////////////////////////////////
                
                /////////////single changes//////////////////
                else if (title == "rotationSpeed"){
                    vector<string> nameValue;
                    nameValue = ofSplitString(received[1], ": ");
                    int index = ofToInt(nameValue[0]);
                    
                    disc.setRotationSpeed(index, ofToFloat(nameValue[1]));
                }
                
                else if (title == "radius"){
                    vector<string> nameValue;
                    nameValue = ofSplitString(received[1], ": ");
                    int index = ofToInt(nameValue[0]);
                    float newValue = ofToFloat(nameValue[1]);
                    float oldValue = disc.getThickness(index);
                    
                    // prepare event record
                    string time = ofToString(ofGetElapsedTimeMillis());
                    string IP = server.getClientIP(i);
                    string parameter = title;
                    string change;
                    if (newValue > oldValue) change = "increase";
                    else if (newValue < oldValue) change = "decrease";
                    
                    if(newValue != oldValue){
                        // set change
                        disc.setThickness(index, newValue);
                        
                        // form event
                        vector<string> newEvent;
                        newEvent.push_back(time);
                        newEvent.push_back(IP);
                        newEvent.push_back(parameter);
                        newEvent.push_back(change);
                        
                        //add to eventList
                        eventList.push_back(newEvent);
                    }
                }
                
                else if (title == "density"){
                    vector<string> nameValue;
                    nameValue = ofSplitString(received[1], ": ");
                    int index = ofToInt(nameValue[0]);
                    int newValue = ofToFloat(nameValue[1]);
                    int oldValue = disc.getDensity(index);
                    
                    // prepare event record
                    string time = ofToString(ofGetElapsedTimeMillis());
                    string IP = server.getClientIP(i);
                    string parameter = title;
                    string change;
                    if (newValue > oldValue) change = "increase";
                    else if (newValue < oldValue) change = "decrease";
                    
                    if(newValue != oldValue){
                        // set change
                        disc.setDensity(index, newValue);
                        
                        // form event
                        vector<string> newEvent;
                        newEvent.push_back(time);
                        newEvent.push_back(IP);
                        newEvent.push_back(parameter);
                        newEvent.push_back(change);
                        
                        //add to eventList
                        eventList.push_back(newEvent);
                    }
                }
                
                else if (title == "texture"){
                    vector<string> nameValue;
                    nameValue = ofSplitString(received[1], ": ");
                    int index = ofToInt(nameValue[0]);
                    int newValue = ofToInt(nameValue[1]);
                    int oldValue = disc.getTexture(index);
                    
                    // prepare event record
                    string time = ofToString(ofGetElapsedTimeMillis());
                    string IP = server.getClientIP(i);
                    string parameter = title;
                    string change = ofToString(newValue);
                    
                    if(newValue != oldValue){
                        // set change
                        disc.setTexture(index, newValue);
                        
                        // form event
                        vector<string> newEvent;
                        newEvent.push_back(time);
                        newEvent.push_back(IP);
                        newEvent.push_back(parameter);
                        newEvent.push_back(change);
                        
                        //add to eventList
                        eventList.push_back(newEvent);
                    }

                    
                    
                }
                
                else if (title == "mute"){
                    vector<string> nameValue;
                    nameValue = ofSplitString(received[1], ": ");
                    int index = ofToInt(nameValue[0]);
                    int newValue = ofToInt(nameValue[1]);
                    int oldValue = disc.isMute(index);
                    
                    // prepare event record
                    string time = ofToString(ofGetElapsedTimeMillis());
                    string IP = server.getClientIP(i);
                    string parameter = title;
                    string change;
                    if (newValue > oldValue) change = "increase";
                    else if (newValue < oldValue) change = "decrease";
                    
                    if(newValue != oldValue){
                        // set change
                        disc.setMute(index, newValue);
                        
                        // form event
                        vector<string> newEvent;
                        newEvent.push_back(time);
                        newEvent.push_back(IP);
                        newEvent.push_back(parameter);
                        newEvent.push_back(change);
                        
                        //add to eventList
                        eventList.push_back(newEvent);
                    }
                }
                
                else if (title == "move"){
                    vector<string> nameValue;
                    nameValue = ofSplitString(received[1], ": ");
                    int index = ofToInt(nameValue[0]);
                    int newValue = ofToInt(nameValue[1]);
                    int oldValue = disc.isMoving(index);
                    
                    
                    // prepare event record
                    string time = ofToString(ofGetElapsedTimeMillis());
                    string IP = server.getClientIP(i);
                    string parameter = title;
                    string change;
                    if (newValue > oldValue) change = "increase";
                    else if (newValue < oldValue) change = "decrease";
                    
                    if(newValue != oldValue){
                        // set change
                        disc.setMoving(index, newValue);
                        
                        // form event
                        vector<string> newEvent;
                        newEvent.push_back(time);
                        newEvent.push_back(IP);
                        newEvent.push_back(parameter);
                        newEvent.push_back(change);
                        
                        //add to eventList
                        eventList.push_back(newEvent);
                    }
                    
                }
                
                else if (title == "moveReset"){
                    int index = ofToInt(received[1]);
                    
                    // prepare event record
                    string time = ofToString(ofGetElapsedTimeMillis());
                    string IP = server.getClientIP(i);
                    string parameter = title;
                    
                    // set change
                    disc.resetPerlin[index] = 1;
                    
                    // form event
                    vector<string> newEvent;
                    newEvent.push_back(time);
                    newEvent.push_back(IP);
                    newEvent.push_back(parameter);
                    
                    //add to eventList
                    eventList.push_back(newEvent);
                }
                
            
                else if (title == "moveAll"){
                    for(int i = 0; i<disc.getDiscIndex(); i++){
                        disc.setMoving(i, 1);
                    }
                }
                
                else if (title == "stopAll"){
                    for(int i = 0; i<disc.getDiscIndex(); i++){
                        disc.setMoving(i, 0);
                    }
                }
                
                else if (title == "resetAll"){
                    for(int i = 0; i<disc.getDiscIndex(); i++){
                        disc.resetPerlin[i] = 1;
                    }
                }
                
                else if (title == "zPosition"){
                    vector<string> nameValue;
                    nameValue = ofSplitString(received[1], ": ");
                    disc.setPosition(ofToInt(nameValue[0]), ofToFloat(nameValue[1]));
                }
                
                else if (title == "zPositionAll"){
                    for (int i = 1; i < received.size()-1; i++) {
                        vector<string> nameValue;
                        nameValue = ofSplitString(received[i], ": ");
                        disc.setPosition(ofToInt(nameValue[0]), ofToFloat(nameValue[1]));
                    }
                }
                
                else if (title == "counter"){
                    vector<string> nameValue;
                    nameValue = ofSplitString(received[1], ": ");
                    disc.setCounter(ofToInt(nameValue[0]), ofToInt(nameValue[1]));
                }
                
                else if (title == "life"){
                    Player *_player = NULL;
                    for(int j = 1; j < received.size(); j++ ){
                        vector<string> playerData = ofSplitString(received[j], ": ");
                        if (playerData[0] == "IP"){
                            for (int k = 0; k < players.size(); k++) {
                                if(playerData[1] == players[k]->getIP()) {
                                    _player = players[k];
                                    break;
                                }
                            }
                        }
                        if (playerData[0] == "lifeChange" && _player != NULL) _player->changeLife(ofToFloat(playerData[1]));
                    }
                }
                
                else if (title == "otherPlayersIndex"){
                    for(int i = 1; i < received.size(); i++ ){
                        vector<string> playerData = ofSplitString(received[i], ": ");
                        int thisPlayer;
                        if (playerData[0] == "IP"){
                            for (int j = 0; j < players.size(); j++) {
                                if(playerData[1] == players[j]->getIP()) {
                                    thisPlayer = j;
                                    break;
                                }
                            }
                        }
                        if (playerData[0] == "index") players[thisPlayer]->setDiscIndex(ofToInt(playerData[1]));
                    }
                }
                
                else if (title == "goodbye"){
                    cout<< str << endl;
                    for (int j = 0; j < players.size(); j++) {
                        if(server.getClientIP(i) == players[j]->getIP()) {
                            players[j]->setConnection(false);
                            players[j]->setDiscIndex(-1);
                        }
                    }
                }
                /////////////////////////////////////////////
            }
            
        }
        else{
            for(int j = 0; j < players.size(); j++){
                if(server.getClientIP(i) == players[j]->getIP()) players[j]->setConnection(false);
            }
//            server.sendToAll("goodbye//"+server.getClientIP(i));
        }
    }
    
    //check first list member and remove if older than 10 seconds
    if( eventList.size() > 0){
        int now = ofGetElapsedTimeMillis();
        int oldest = ofToInt(eventList[0][0]);
        if(now - oldest > 10000) eventList.erase(eventList.begin());
    }
}


//--------------------------------------------------------------
void ofApp::draw(){
    
    ofBackgroundGradient(255, 233);
    
    ofSetColor(0);
    ofDrawBitmapString("Server Running", 10, 20);
    int online = 0;
    for(int i = 0; i < players.size(); i++){
        if(players[i]->isConnected()) online++;
    }
    ofDrawBitmapString("Number of Players: "+ofToString(online), 10, 40);
    
    
    //Add names of online players//
    
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}

//--------------------------------------------------------------

void ofApp::eventMatch(string IP, string parameter, string change){
    
    for (int i = 0; i < eventList.size(); i++) {
        
        //check if other players recently changed the same parameter the same way
        if(IP != eventList[i][1] && parameter == eventList[i][2] && change == eventList[i][2]){
            
            //reward imitating player
            string lifeUpdate = "life//";
            lifeUpdate += "IP: "+ IP + "//";
            lifeUpdate += "lifeChange: "+ofToString(reward) + "//";
            server.sendToAll(lifeUpdate);
            
            //reward imitated player
            lifeUpdate = "life//";
            lifeUpdate += "IP: "+ eventList[i][1] + "//";
            lifeUpdate += "lifeChange: "+ofToString(reward) + "//";
            server.sendToAll(lifeUpdate);
            
            
        }
    }
    
}

