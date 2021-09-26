We should have a test feature that can test Comp mode or only auton and stuff.
Controller should print two seperate things if debug mode is on.

/**
 * A loop for printing things to the controller
 */
void controller_screen_loop() {
    while(true) {
        if(!disableControllerScreenLoop) {
        	if(DEBUG){
	            master.clear();
	            // Print out debug info
        	}else{
        		// Print out modes and other useful comp. stuff
        	}
        }
        pros::delay(50);
    }
}