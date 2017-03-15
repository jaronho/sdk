/**********************************************************************
* Author��jaron.ho
* Date��2009-9-23 19:00
* Brief���ֽ�����(����Э�����л�)
**********************************************************************/
#ifndef _BYTE_ARRAY_H_
#define _BYTE_ARRAY_H_

#include <string>

class ByteArray {
public:
	ByteArray();
	virtual ~ByteArray();

public:
	static short swab16(short x);											// ��С��ת��(����������תΪ����������)
	static short swab16_array(char* pBuf);									// ��С��ת��(�ֽ���תΪ����������)

	static int swab32(int x);												// ��С��ת��(��������תΪ��������)
	static int swab32_array(char* pBuf);									// ��С��ת��(�ֽ���תΪ��������)

	static int max_size(void);												// ������Ϣ��󳤶�

public:
	void print(void);														// ��ӡ��ǰ����
	void reuse(void);														// ���,����ʹ�õ�ǰ�ֽ���
	int getCurLength(void);													// ��ȡ�ֽ�����ǰ����
	const char* getContent(void);											// ��ȡ�ֽ�������
	bool setContent(const char* content, int len);							// �����ֽ�������
	int space(void);														// ʣ���д�ռ�
	bool copy(const char* buf, int n);										// ����
	
public:
	short read_int16(void);													// ���ֽ�����ȡ16λ����
	bool write_int16(short value);											// ���ֽ���д16λ����
	
	unsigned short read_uint16(void);										// ���ֽ�����ȡ16λ�޷�������
	bool write_uint16(unsigned short value);								// ���ֽ���д16λ�޷�������
	
	int read_int(void);														// ���ֽ�����ȡ32λ����
	bool write_int(int value);												// ���ֽ���д32λ����
	
	unsigned int read_uint(void);											// ���ֽ�����ȡ32λ����
	bool write_uint(unsigned int value);									// ���ֽ���д32λ�޷�������

	long read_long(void);													// ���ֽ�����ȡ������
	bool write_long(long value);											// ���ֽ���д������
	
	unsigned long read_ulong(void);											// ���ֽ�����ȡ�޷��ų�����
	bool write_ulong(unsigned long value);									// ���ֽ���д�޷��ų�����

	long long read_int64(void);												// ���ֽ�����ȡ64λ����
	bool write_int64(long long value);										// ���ֽ���д64λ����
	
	unsigned long long read_uint64(void);									// ���ֽ�����ȡ64λ�޷�������
	bool write_uint64(unsigned long long value);							// ���ֽ���д64λ�޷�������

	float read_float(void);													// ���ֽ�����ȡ������
	bool write_float(float value);											// ���ֽ���д������
	
	double read_double(void);												// ���ֽ�����ȡ˫������
	bool write_double(double value);										// ���ֽ���д˫������

	bool read_bool(void);													// ���ֽ�����ȡBoolֵ
	bool write_bool(bool value);											// ���ֽ���д������

	char read_char(void);													// ���ֽ�����ȡ�ַ�
	bool write_char(char value);											// ���ֽ���д�ַ���
	
	unsigned char read_uchar(void);											// ���ֽ�����ȡ�޷����ַ�����
	bool write_uchar(unsigned char value);									// ���ֽ���д�޷����ַ�������

	unsigned char* read_string(unsigned char* info, unsigned int len);		// ���ֽ�����ȡ�ַ���
	bool write_string(const char* str);										// ���ֽ���д�ַ���
	
	std::string read_string(void);											// ���ֽ�����ȡ�ַ���
	bool write_string(const std::string& str);								// ���ֽ���д�ַ���

private:
	char* rd_ptr(void);
	char* wr_ptr(void);
	
	bool rd_ptr(int n);
	bool wr_ptr(int n);

private:
	char* m_pContent;			// �ֽ�������ָ��
	int m_nTotalSize;			// �ֽ��������С
	int m_nRdptr;				// ��ȡλ��
	int m_nWrPtr;				// д��λ��
};

#endif	// _BYTE_ARRAY_H_