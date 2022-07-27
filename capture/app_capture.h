#ifndef APP_CAPTURE_H
#define APP_CAPTURE_H

/**@brief  ץ�������ʼ��
 * @return �ɹ�����OK��ʧ�ܷ���ERROR
 */
int app_capture_init();

/**@brief  ץ���������
 * @return �ɹ�����OK��ʧ�ܷ���ERROR
 */
int app_capture_uninit();

/**@brief  ��ʼץ��
 * @param  netName�� Ҫץ������������ ip ����
 * @param  ip�� ���˵�ip��ֻ�д�IPΪԴ��Ŀ��ʱץ��
 * @return �ɹ�����OK��ʧ�ܷ���ERROR
 */
int app_capture_start(char* netName, unsigned int ip);

/**@brief  ֹͣץ�����˺������������3s
 * @param  ��
 * @return ��
 * @see
 * @note
 */
void app_capture_stop();

#endif // APP_CAPTURE_H