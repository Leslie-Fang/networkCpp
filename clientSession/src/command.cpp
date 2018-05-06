//
// Created by leslie-fang on 2018/5/6.
//
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "command.h"

bool ParseCommand(char * raw_command){
    return true;
}

void ParseInput(commands & myCommand, char * raw_command){
    int inputLength = strlen(raw_command);
    int seperator=0; //between command and the arguments
    for(int i=0;i<inputLength;i++){
        if(raw_command[i] == ' '){
            seperator = i;
            break;
        }
    }
    for(int i=0;i<seperator;i++){
        myCommand.command[i] = raw_command[i];
    }
    myCommand.command[seperator] = '\0';
    for(int i=seperator+1;i<inputLength;i++){
        myCommand.arguments[i-seperator-1] = raw_command[i];
    }
    myCommand.arguments[inputLength-seperator-1] = '\0';
    return;
}

void newCommand(commands & myCommand,char * raw_command){
    int inputLength = strlen(raw_command);
    int seperator=0;
    for(int i=0;i<inputLength;i++){
        if(raw_command[i] == ' '){
            seperator = i;
            break;
        }
    }
    myCommand.command = new char[seperator+1];
    myCommand.arguments = new char[inputLength-seperator];
    return;
}

void freeCommand(commands & myCommand){
    delete [] myCommand.command;
    delete [] myCommand.arguments;
    return;
}

void GenerateCommand(commands & myCommand,char * buf,int inputLength){
    //int inputLength = 3+strlen(myCommands.command)+3+myCommands.arguments+3+2;
    int cLength = 0;
    unsigned char protocol_head[3]={'[','/',']'};
    unsigned char protocol_middle[3]={'[','^',']'};
    unsigned char protocol_sm[3]={'[','_',']'};
    unsigned char protocol_last[3]={'[','>',']'};
    //char sendBuf[inputLength-2];
    memcpy(buf+cLength,protocol_head,3);
    cLength +=3;
    memcpy(buf+cLength,myCommand.command,strlen(myCommand.command));
    cLength += strlen(myCommand.command);
    memcpy(buf+cLength,protocol_middle,3);
    cLength += 3;
    memcpy(buf+cLength,myCommand.arguments,strlen(myCommand.arguments));
    cLength += strlen(myCommand.arguments);
    memcpy(buf+cLength,protocol_sm,3);
    cLength +=3;
    short crc_result = crc16(buf,inputLength-5);
    memcpy(buf+cLength,&crc_result,2);
    cLength +=2;
    memcpy(buf+cLength,protocol_last,3);
    return;
}

unsigned short crc_update (unsigned short crc, unsigned char data){
    data ^= (crc & 0xff);
    data ^= data << 4;
    return ((((unsigned short )data << 8) | ((crc>>8)&0xff)) ^ (unsigned char )(data >> 4)
            ^ ((unsigned short )data << 3));
}

unsigned short crc16(void* data, unsigned short cnt){
    unsigned short crc=0xff;
    unsigned char * ptr=(unsigned char *) data;
    int i;
    for (i=0;i<cnt;i++){
        crc=crc_update(crc,*ptr);
        ptr++;
    }
    return crc;
}