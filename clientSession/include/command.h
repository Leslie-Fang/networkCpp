//
// Created by leslie-fang on 2018/5/6.
//

#ifndef NETWORKCPP_COMMAND_H
#define NETWORKCPP_COMMAND_H
//raw_command是客户端传输过来的原始命令
//raw_command格式为[\]command[^]arguments[_]crc校验结果[>]
//开头[\]
//command：目前只支持set和get
//[^]:后面跟参数
//arguments: 目前只支持键值对： 如果是键值对就是 a(key) b(value)
//[_]:后面跟CRC校验结果
//crc校验结果：客户端会算一次值传输过来，服务器端再算一次，如果两个值不一样索命有丢包，丢弃命令并返回错误给客户端
//结尾[>]
struct commands{
    char * command;
    char * arguments;
};
bool ParseCommand(char * raw_command);//parse the commands from remote client/server
void GenerateCommand(commands & myCommand,char * buf,int inputLength);//encode the commands into the required format
void ParseInput(commands & myCommand, char * raw_command); //parse the input of user from the session
void newCommand(commands & myCommand,char * raw_command);
void freeCommand(commands & myCommand);
unsigned short crc_update (unsigned short crc, unsigned char data);
unsigned short crc16(void* data, unsigned short cnt);
#endif //NETWORKCPP_COMMAND_H
