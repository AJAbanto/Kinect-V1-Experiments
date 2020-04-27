#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NuiApi.h"

//Kinect sensor
INuiSensor * sensor;

//Handle for kinect sensor
HANDLE Skel_dready;


void check_skel_data(); //function to get actual skeleton data
void cleanup();		//for closing sensor and handle variables

int main() {

	int sensor_cnt;		//for number of kinects
	
	//Checking for sensor number
	if (NuiGetSensorCount(&sensor_cnt) < 0 || sensor_cnt < 1) {
		printf("No Kinects found\n");
		return -1;		 
	}

	printf("Kinect sensors connected: %d\n\n", sensor_cnt);		//Check kinect sensor count

	//Connect to kinect found (only first one)
	HRESULT hr = NuiCreateSensorByIndex(0, &sensor);
	
	if (FAILED(hr)) {
		printf("Failed to connect to kinect\n");
		return -1;
	}
	else
		printf("Kinect object created\n");
	
	
	//Initialize kinect for skeleton tracking
	sensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_SKELETON);

	Skel_dready = CreateEvent(NULL, TRUE, FALSE, NULL);	//Check what this means after besides it creates and event handler
	sensor->NuiSkeletonTrackingEnable(Skel_dready, 0);



	
	while (1) {

		//wait for available data
		if (WAIT_OBJECT_0 == WaitForSingleObject(Skel_dready, 0)) {
			check_skel_data();	//get one set of data
		}
		
		//check if exit key (F) was pressed
		short F_key = GetAsyncKeyState(0x01);

		//mask bits, if MSB is 1 then it was pressed so exit loop 
		if ((F_key & 0x8000) == 0x8000) {
			printf("Exit key was pressed\n");
			break;
		}
	}


	cleanup();

	return 0;
}


//actually get the skeleton data
void check_skel_data(){
	NUI_SKELETON_FRAME skeletonFrame = { 0 };

	//get skeleton fram data
	HRESULT hr = sensor->NuiSkeletonGetNextFrame(0, &skeletonFrame);
	if (FAILED(hr)) {
		printf("Couldn't get next frame\n");
		return;
	}

	//smoothen out skeletonfram data
	sensor->NuiTransformSmooth(&skeletonFrame, 0);

	//Transfer to proper variable
	NUI_SKELETON_DATA skeletonData = skeletonFrame.SkeletonData[0];

	if(skeletonData.eTrackingState == NUI_SKELETON_NOT_TRACKED) {
		printf("No skeleton tracked\n");
		return;
	}
	else {
		//assume that head is tracked so we get the data
		Vector4 head_pos = skeletonData.SkeletonPositions[NUI_SKELETON_POSITION_HEAD];
		Vector4 left_hand_pos = skeletonData.SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT];
		printf("H(x: %f , y: %f ,z: %f) | LH( x: %f , y: %f , z: %f )\n", head_pos.x, head_pos.y,head_pos.z,left_hand_pos.x ,left_hand_pos.y,left_hand_pos.z);
	}

	
}


//handle resources
void cleanup() {
	//Clean up
	sensor->Release();			//Release sensor (might cause some detection errors if not)
	CloseHandle(Skel_dready);	//Close handle
}