#ifndef APP_CAPTURE_H
#define APP_CAPTURE_H

/**@brief  抓包组件初始化
 * @return 成功返回OK，失败返回ERROR
 */
int app_capture_init();

/**@brief  抓包组件析构
 * @return 成功返回OK，失败返回ERROR
 */
int app_capture_uninit();

/**@brief  开始抓包
 * @param  netName： 要抓包的网卡名字 ip 过滤
 * @param  ip： 过滤的ip，只有此IP为源或目的时抓包
 * @return 成功返回OK，失败返回ERROR
 */
int app_capture_start(char* netName, unsigned int ip);

/**@brief  停止抓包，此函数会阻塞最大3s
 * @param  无
 * @return 无
 * @see
 * @note
 */
void app_capture_stop();

#endif // APP_CAPTURE_H