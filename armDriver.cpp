#include "armDriver.h"

ArmDriver::ArmDriver(char* portName){
    cout << "Initialize " << portName << "... ";
    portDescriptor = Uart::initUart(portName, __9600, DATA_8_BIT, STOP_1_BIT, NO_PARITY);
    cout << "[SUCCESS]" << endl;
    pthread_mutex_init(&serialPortReaderMutex, NULL);
    pthread_mutex_init(&serialPortWriterMutex, NULL);

    initArmStatus();
}

void ArmDriver::initArmStatus(){
    memset(&actionData.bytes, 0, sizeof(actionData.bytes));
    actionData.floatData[0] = (float) AXIS;
    actionData.floatData[1] = (float) IDLE;
    actionData.floatData[7] = 50;
}

void ArmDriver::connectToArm(){
    pthread_create(&serialPortReaderThread, NULL, readSerialPort, this );
    // Fucking misery BUG
    // Need to wait dobot finishing initialization then we can send message to it !
    sleep(5);
    pthread_create(&serialPortWriterThread, NULL, writeSerialPort, this );
}

void *ArmDriver::readSerialPort(void *arg){
    uint8_t buffer[1];
    ProtoclData dataBuffer;
    uint8_t index = 0;
    ArmDriver *driver = (ArmDriver *)arg;
    cout << "Starting listening..." << endl;
    while(1){
        if ( read(driver->portDescriptor,buffer,1) == -1 ){
            perror("readSerialPort()");
        }
        if ( buffer[0] == PROTOCOL_HEADER){
            index = 0;
        }
        else if (buffer[0] == PROTOCOL_FOOTER && index == PROTOCOL_DATA_SIZE ){
            pthread_mutex_lock(&driver->serialPortReaderMutex);
            driver->armStatus.coordinateX = dataBuffer.floatData[0];
            driver->armStatus.coordinateY = dataBuffer.floatData[1];
            driver->armStatus.coordinateZ = dataBuffer.floatData[2];
            driver->armStatus.headRotation = dataBuffer.floatData[3];
            driver->armStatus.baseAngle = dataBuffer.floatData[4];
            driver->armStatus.rearArmAngle = dataBuffer.floatData[5];
            driver->armStatus.foreArmAngle = dataBuffer.floatData[6];
            driver->armStatus.servoAngle = dataBuffer.floatData[7];
            driver->armStatus.isGrabbed = dataBuffer.floatData[8];
            driver->armStatus.gripperAngle = dataBuffer.floatData[9];
            pthread_mutex_unlock(&driver->serialPortReaderMutex);
            system("clear");
            cout << getArmDataString(driver->armStatus) << endl;
        }
        else{
            if (index < PROTOCOL_DATA_SIZE){
                dataBuffer.bytes[index++] = buffer[0];
            }
        }
    }
    pthread_exit(NULL);
}

void *ArmDriver::writeSerialPort(void *arg){
    uint8_t buffer[PROTOCOL_DATA_SIZE+2];
    ArmDriver *driver = (ArmDriver *)arg;

    cout << "Starting writing..." << endl;
    while(1){
        buffer[0] = PROTOCOL_HEADER;
        buffer[PROTOCOL_DATA_SIZE+1] = PROTOCOL_FOOTER;
        pthread_mutex_lock(&driver->serialPortWriterMutex);
        memcpy(buffer+1, &driver->actionData.bytes, PROTOCOL_DATA_SIZE);
        pthread_mutex_unlock(&driver->serialPortWriterMutex);
        if ( write(driver->portDescriptor, buffer, sizeof(buffer)) == -1){
            perror("writeSerialPort()");
        }
        usleep(500000);
    }
    pthread_exit(NULL);
}

ArmStatus ArmDriver::getArmStatus(){
    ArmStatus armStatusCopy;
    pthread_mutex_unlock(&serialPortReaderMutex);
    armStatusCopy = armStatus;
    pthread_mutex_unlock(&serialPortReaderMutex);

    return armStatusCopy;
}

string ArmDriver::getArmDataString(ArmStatus armStatus){
    stringstream ss;

    ss << "coordinatX = " << armStatus.coordinateX <<endl;;
    ss << "coordinatY = " << armStatus.coordinateY <<endl;
    ss << "coordinatZ = " << armStatus.coordinateZ <<endl;
    ss << "headRotation = " << armStatus.headRotation <<endl;
    ss << "baseAngle = " << armStatus.baseAngle <<endl;
    ss << "rearArmAngle = " << armStatus.rearArmAngle <<endl;
    ss << "foreArmAngle = " << armStatus.foreArmAngle <<endl;
    ss << "servoAngle = " << armStatus.servoAngle <<endl;
    ss << "pumpState = " << armStatus.isGrabbed <<endl;
    ss << "gripperAngle = " << armStatus.gripperAngle <<endl;
    ss << "oldX = "<< armStatus.oldX <<endl;
    ss << "oldY = "<< armStatus.oldY <<endl;
    return ss.str();
   
};

void ArmDriver::changeArmStatus(MotionType motionType, Action action, float startVelocity){
    pthread_mutex_lock(&serialPortWriterMutex);
    actionData.floatData[0] = (float) motionType;
    actionData.floatData[1] = (float) action;
    actionData.floatData[7] = (float) startVelocity;
    pthread_mutex_unlock(&serialPortWriterMutex);
}

/*add by wayne 0714*/
//have limit bug
void ArmDriver::linearMode(float X, float Y)
{
   
    armStatus.oldX = armStatus.coordinateX;
    armStatus.oldY = armStatus.coordinateY;
    if(X > 0)
	changeArmStatus(LINEAR, (Action)1, 10);		
    else
	changeArmStatus(LINEAR, (Action)2, 10);
    while(armStatus.coordinateX - armStatus.oldX <= X)
	{
	printf("%lf\n",armStatus.coordinateX - armStatus.oldX);
	}
	if(X < 0)
	{	
	    while(armStatus.oldX- armStatus.coordinateX <= -X)
	    {
		printf("%lf\n",armStatus.oldX- armStatus.coordinateX);
	    }
	} 
    changeArmStatus(LINEAR, (Action)0, 10);
    if(Y > 0)		
    	changeArmStatus(LINEAR, (Action)3, 10);
    else
	changeArmStatus(LINEAR, (Action)4, 10);

    while(armStatus.coordinateY - armStatus.oldY <= Y)
	{
	printf("%lf\n",armStatus.coordinateY - armStatus.oldY);
	}
    if(Y < 0)
    {
    	while(armStatus.oldY- armStatus.coordinateY <= -Y)
	    {
		printf("%lf\n",armStatus.oldY- armStatus.coordinateY);
	    }
    }
    changeArmStatus(LINEAR, (Action)0, 10);
 
}

void ArmDriver::moveMode(float X, float Y, float Z ,float mode){
    pthread_mutex_lock(&serialPortWriterMutex);
    actionData.floatData[0] = (float) mode;
    actionData.floatData[2] = (float) X;
    actionData.floatData[3] = (float) Y; 
    actionData.floatData[4] = (float) Z;
    actionData.floatData[6] = (float) 1; 
    actionData.floatData[7] = (float) 1;
//    actionData.floatData[9] = (float) 5;
    pthread_mutex_unlock(&serialPortWriterMutex);
//    changeArmStatus(LINEAR, (Action)9, 10);
}
/*
void ArmDriver::angleMode(float base, float rear, float fore, float mode)
{
    pthread_mutex_lock(&serialPortWriterMutex);
    actionData.floatData[0] = (float) 3;
    actionData.floatData[2] = (float)base;
    actionData.floatData[3] = (float) rear; 
    actionData.floatData[4] = (float) fore;
    actionData.floatData[7] = (float) mode;
//    actionData.floatData[9] = (float) 5;
    pthread_mutex_unlock(&serialPortWriterMutex);
}*/
