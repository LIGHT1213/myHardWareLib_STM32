#ifndef SHELL_PORT_H
#define SHELL_PORT_H
#include "main.h"
#include "usart.h"
#include "shell.h"
#include "cmsis_os.h"
/**
 * @brief shell读取数据函数原型
 *
 * @param char shell读取的字符
 *
 * @return char 0 读取数据成功
 * @return char -1 读取数据失败
 */
void shellWrite(char data);
signed char  shellRead(char *data);

extern Shell shell;
/**
 * @brief shell写数据函数原型
 *
 * @param const char 需写的字符
 *
 */
 
extern osSemaphoreId shellBinarySemHandle;
#endif