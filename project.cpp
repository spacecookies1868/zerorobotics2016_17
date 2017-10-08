//Begin page main
//Global Variables
float zoneInfo[4];  //Where your zone is

float dropOff[3][3];//Where you drop off the SPSs
int at, tarCount;  //Which drop off location
float myState[12];
int times, gotoFirstItem; //Use blue values and multiple by times
float att[3], myAtt[3], v1[3], v2[3];
float goTo[3];
bool holding, keepStill;
bool taken[6];
float facingVariance, myPos[3],itemPos[3],itemState[12],vectorBetween[3], fuel;
int holdingItem,count;
int itemID;

void init() {
    game.dropSPS(); //first SPS dropped
    keepStill = false;
    tarCount = 60;
    gotoFirstItem = 0;
    count=0;
    at = 1;
    
    api.getMyZRState(myState);
    game.getItemZRState(itemState, gotoFirstItem);
    
    for (int i =0; i < 3; i++){
        myPos[i]=myState[i];
        myAtt[i]=myState[i+6];
    }
    
    if (myState[1]<0 ) { //RED SPHERE
        times=-1;
        gotoFirstItem=1;
    } else {
        times = 1;
    }  //BLUE SPHERE

    dropOff[1][0] = 0.158*times;
    dropOff[1][1] = 0.208*times;
    dropOff[1][2] = 0.158*times;

    dropOff[2][0] = -0.6*times;
    dropOff[2][1] = 0.6*times;
    dropOff[2][2] = -0.23*times;
    
    mathVecSubtract(vectorBetween, itemState, myState, 3);
    for (int i=0; i < 3; i++){
        att[i]=vectorBetween[i];
    }

    api.setAttitudeTarget(att);
    //DEBUG(("att: <%f, %f, %f>", att[0], att[1], att[2]));
    
    for (int i=0; i<6; i++) {
        taken[i] = false;
    }
    holding = false;
}

void loop(){
    if(count>11)  {
        dropOff[1][0] = 0.1188*times;
        dropOff[1][1] = 0.1913*times;
        dropOff[1][2] = 0.1188*times;    
    } 
    if(at==1 && gotoFirstItem ==-1){
        dropOff[1][0] = 0.3*times;
        dropOff[1][1] = 0.6*times;
        dropOff[1][2] = 0.2*times;     
    }

    if(keepStill){
        stayInZone();
        return;
    }
    count++;
    holdingItem = -1;
    api.getMyZRState(myState);
    for (int i =0; i < 3; i++){
        myPos[i]=myState[i];
    }    
    //DEBUG(("myInfo: <%f, %f, %f>", myState[0], myState[1], myState[2]));

    if (!SPS()) {
        //All SPSs are placed
        game.getZone(zoneInfo);
        //DEBUG(("rawZoneInfo: <%f, %f, %f, %f>", zoneInfo[0], zoneInfo[1], zoneInfo[2],zoneInfo[3]));
        //Change ZoneInfo
        if (!goToZone()) {
            //It's not holding an item
            pickAnItem();
        }
        for (int i=0; i<6; i++) {
            if(i<3){
                myPos[i] = myState[i];
            }
            if (game.hasItem(i) == 1) {
                holdingItem = i;
                //DEBUG(("Holding Item %d", i));
            }
            if(game.itemInZone(i)){
                //DEBUG(("Item in zone %d",i));
            }
        }
        
        if(holdingItem>-1){
            game.getItemZRState(itemState, holdingItem);
            for (int i=0; i<3; i++) {
                itemPos[i] = itemState[i];
            }
            //DEBUG(("goTo: <%f, %f, %f>", zoneInfo[0], zoneInfo[1], zoneInfo[2]));
           // DEBUG(("vectorBetween: <%f, %f, %f>", vectorBetween[0], vectorBetween[1], vectorBetween[2]));
            mathVecSubtract(vectorBetween, itemPos, myPos, 3);
            mathVecSubtract(goTo, zoneInfo, vectorBetween, 3);
           // DEBUG(("goTo: <%f, %f, %f>", goTo[0], goTo[1], goTo[2]));
        }else{
            api.setAttitudeTarget(att);
        }
    }

    //DEBUG(("Att: <%f, %f, %f>", att[0], att[1], att[2]));

    //DEBUG(("goTo: <%f, %f, %f>", goTo[0], goTo[1], goTo[2]));

    //DEBUG(("zoneInfo#5: <%f, %f, %f>", zoneInfo[0], zoneInfo[1], zoneInfo[2]));

    
    api.setPositionTarget(goTo);

} //end loop()

bool SPS() {
    if (at >= 3) {return false;}
    if(gotoFirstItem>-1){
        //DEBUG(("try docking item %d", gotoFirstItem));
        if(dock(gotoFirstItem)){
            
            gotoFirstItem = -1;
            tarCount = count+14;
        }
    }else if(count==tarCount-4) {
        at ++;
    }else if(count==tarCount && at==2) {    
        game.dropSPS();
        
    }else if(count==tarCount+10 ) {
        at++;
        game.dropSPS();
        return false;
    }
    if(at==1 && gotoFirstItem ==-1){
        dropOff[1][0] = 0.3*times;
        dropOff[1][1] = 0.6*times;
        dropOff[1][2] = 0.2*times;     
    }    
    goTo[0] = dropOff[at][0];
    goTo[1] = dropOff[at][1];
    goTo[2] = dropOff[at][2];
    
    if(gotoFirstItem)
        api.setAttitudeTarget(att);
    return true;
} //end SPS()

bool equal (float a, float b) {
    if (a+0.01 >= b && a-0.01 <= b) return true;
    return false;
}

bool goToZone() {
    float dist;
    fuel = game.getFuelRemaining();
    //DEBUG(("Fuel running low %f",fuel));
    //returns false if it's already in the zone or not holding an item
    drop();
    if (holding == false) {return false;}
    //mathVecSubtract(v1, itemState, myState, 3);
    mathVecSubtract(att, zoneInfo, itemState, 3);
    dist = mathVecMagnitude(att, 3);
    if(dist<0.3){
        goTo[0] = myState[0]+att[0];
        goTo[1] = myState[1]+att[1];
        goTo[2] = myState[2]+att[2];
    }else{
        goTo[0] = zoneInfo[0];
        goTo[1] = zoneInfo[1];
        goTo[2] = zoneInfo[2];        
    
        att[0] = zoneInfo[0];
        att[1] = zoneInfo[1];
        att[2] = zoneInfo[2];
    }

    return true;
} //end goToZone()

float distanceTo(int item) {
    float pos[3];
    game.getItemLoc(pos, item);
    float x_change = (pos[0]-myState[0]) * (pos[0]-myState[0]);
    float y_change = (pos[1]-myState[1]) * (pos[1]-myState[1]);
    float z_change = (pos[2]-myState[2]) * (pos[2]-myState[2]);
    return sqrtf(x_change + y_change + z_change);
} //end distanceTo

bool dock (int item) {
    //DEBUG(("trying to dock"));
    bool a = false;
    //if (equal(myState[0], goTo[0]) && equal(myState[1], goTo[1]) && equal(myState[2], goTo[2])) {
    //if (canDock(myPos, item)) {
    if (canDockNew(myPos,item)) {
        a = game.dockItem(item);
        if (!a) {
            //DEBUG(("NOT DOCKED"));
        } else {
            holding = true;
        }
    }
    return a;
} //end dock()

bool canDockNew(float myPos[3],int itemID){
    float itemPos[3], deviance[3];
    float dist, devianceMag, devianceLimit = 0.24;
    float dockUpper, dockLower;
    float itemState[12];
    game.getItemZRState(itemState, itemID);
    api.getMyZRState(myState);
    
    for (int i=0; i<3; i++){
        itemPos[i]=itemState[i];
        myPos[i]=myState[i];
        myAtt[i] = myState[i+6];
    }
    if (itemID == 0 || itemID==1) {
        dockUpper=0.173006;
        dockLower=0.151;
    }else if (itemID == 2 || itemID == 3) {
        dockUpper=0.160006;
        dockLower=0.138;
    }else {
        dockUpper=0.146006;
        dockLower=0.124;
    } 
    mathVecSubtract(vectorBetween, itemPos, myPos, 3);
    dist = mathVecMagnitude(vectorBetween, 3);
    //DEBUG(("dockLower, dockHigher, dist %f,  %f, %f", dockLower, dockUpper, dist));
    //DEBUG(("myPos, itemPos, %f, %f, %f,,,%f, %f, %f", myPos[0], myPos[1], myPos[2], itemPos[0], itemPos[1], itemPos[2]));
    
    mathVecNormalize(vectorBetween, 3);
    mathVecNormalize(myAtt, 3);
    mathVecSubtract(deviance, vectorBetween, myAtt, 3);
    devianceMag = mathVecMagnitude(deviance, 3);
    //DEBUG(("deviance %f",devianceMag));
    bool oktodock = dist >= dockLower && dist <= dockUpper && devianceMag<devianceLimit;
    //if (oktodock)
        //DEBUG(("oktodock"));
    return oktodock;
}
void stayInZone(){
    game.getZone(zoneInfo);
    float zonePos[3];
    for (int i=0; i<3; i++){
        zonePos[i]=zoneInfo[i];
    }
    api.setPositionTarget(zonePos);
}
bool closeToZone(){
    game.getZone(zoneInfo);
    float itemPos[3];
    float dist, errorFactor;
    errorFactor = 1;
    if(zoneInfo[3]>0.9){
        errorFactor = zoneInfo[3]/0.88;
    }
    float variance=0.05/errorFactor;
    for (int i=0; i<3; i++){
        itemPos[i]=itemState[i];
    }
    mathVecSubtract(vectorBetween, itemPos, zoneInfo, 3);
    dist = mathVecMagnitude(vectorBetween, 3);
    return (dist < variance);
}
void drop() {
    int item= -1;
    
    for (int i=0; i<6; i++) {
        if (game.hasItem(i) == 1) {
            taken[i] = true;
            item = i;
           // DEBUG(("Holding Item %d", i));
        }
    }
    if(item<0){return;}
    game.getItemZRState(itemState, item);
    //DEBUG(("zoneInfo %f,%f,%f, itemInfo  %f,%f,%f,",zoneInfo[0],zoneInfo[1],zoneInfo[2],itemState[0],itemState[1],itemState[2]));
    if (closeToZone()) {
    //if (equalZone(itemState[0], zoneInfo[0]) && equalZone(itemState[1], zoneInfo[1]) && equalZone(itemState[2], zoneInfo[2])) {
       //DEBUG(("Dropping item======="));
       //DEBUG(("zoneInfo %f,%f,%f, itemInfo  %f,%f,%f,",zoneInfo[0],zoneInfo[1],zoneInfo[2],itemState[0],itemState[1],itemState[2]));
        game.dropItem();
        if(game.itemInZone(item)){
            //DEBUG(("Succeed! dropped in zone"));
            holding = false;
            if(game.getFuelRemaining()<15){
                keepStill = true;
                stayInZone();
            }
        }else{
            //DEBUG(("Failed drop item in zone"));
            if(game.dockItem(item)){
                //DEBUG(("Docked Again!"));
            }else{
                //DEBUG(("Failed Docked Again!"));
                holding = false;
            }
        }
        
    }
} //end drop()
float getItemMutiplier(int item){
    if(item<2) return 18;
    if(item<4) return 2;
    else return 1;
}
void pickAnItem() {
    int go = -1;
    float pickfactor = 0.0;
    float timeLeft =180-game.getCurrentTime();

    for (int i=0; i<6; i++) {
        int multiplier=getItemMutiplier(i);
        //DEBUG(("Distance to %d is %f", i, distanceTo(i)));
        //if ( (taken[i] == false) && (game.hasItem(i) == 0) && (multiplier*timeLeft/distanceTo(i) >pickfactor) && (!game.itemInZone(i)) ) {
        if ( (game.hasItem(i) == 0) && (multiplier*timeLeft/distanceTo(i) >pickfactor) && (!game.itemInZone(i)) ) {
            go = i;
            pickfactor = multiplier*timeLeft/distanceTo(i);
        }
    }
    if (go == -1) {
        return;
    }
    game.getItemLoc(goTo, go);
    att[0] = goTo[0];
    att[1] = 0.0;
    att[2] = 0.0;
    float edit;
    if (go == 0 || go==1) {
        edit = 0.162;
    }else if (go == 2 || go == 3) {
        edit = 0.149;
    }else {
        edit = 0.135;
    }
    //readyToDock(goTo, myPos, goTo, go);
    if (goTo[0] < myState[0]) { 
        goTo[0] += edit; 
        att[0] = -1.0;
    }else { 
        goTo[0] -= edit; 
        att[0] = 1.0;
    }
    //if (goTo[1] < myState[1]) { goTo[1] += edit; }

    //else { goTo[1] -= edit; }
    //DEBUG(("Going to item %d", go));
    itemID=go;
    dock(go);
} //end pickAnItem()
//End page main
