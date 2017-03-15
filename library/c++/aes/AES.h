/**********************************************************************
* Author:	jaron.ho
* Date:		2013-11-17
* Brief:	AES algorithm
**********************************************************************/
#ifndef _AES_H_
#define _AES_H_

class AES
{
public:
	/*
	��	�ܣ����캯��
	��	����key - ��Կ
	����ֵ���޷���ֵ
	*/
	AES(unsigned char* key);
	virtual ~AES();

public:
	/*
	��	�ܣ����ܵ����ַ���
	��	����input - Ҫ���ܵ��ַ���
	����ֵ��unsigned char*
	*/
	unsigned char* encrypt(unsigned char* input);

	/*
	��	�ܣ����ܵ����ַ���
	��	����input - Ҫ���ܵ��ַ���
	����ֵ��unsigned char*
	*/
	unsigned char* decrypt(unsigned char* input);

	// �����Ѿ�ʵ����AES��������ܵ�ԭ��,��ʹ�õ�ʱ��һ�㴦������ַ�����,������ֱ�Ӵ���128λ������,����Ҫ��װһ�¶��ⲿ���ݵļӽ��ܴ���

	/*
	��	�ܣ��������ݣ�����Ƕ������ı����ݽ��м���,��ѡ��˺�����
	��	����input - Ҫ���ܵ�����,����ʱ������ָ��ҪԤ����16�������ֽڵĿռ�,��Ϊ���ܲ���ֱ���޸�ԭ����,����128λ��������ڴ����
			length - Ҫ���ܵ����ݳ���,���ʹ��Ĭ��ֵ,����Ϊ�ַ�������,��'\0'Ϊ��β���㳤��
	����ֵ��void*
	*/
	void* encrypt(void* input, int length = 0);

	/*
	��	�ܣ��������ݣ�����Ƕ�����ı����ݽ��н���,��ѡ��˺�����
	��	����input - Ҫ���ܵ����ݣ�length - 
	����ֵ��void*
	*/
	void* decrypt(void* input, int length);

private:
	void keyExpansion(unsigned char* key, unsigned char w[][4][4]);		// ��Կ��չ
	unsigned char ffMul(unsigned char a, unsigned char b);				// ������GF(2^8)�ϵĳ˷�

	void subBytes(unsigned char state[][4]);							// �ֽ����
	void shiftRows(unsigned char state[][4]);							// ����λ
	void mixColumns(unsigned char state[][4]);							// �л���
	void addRoundKey(unsigned char state[][4], unsigned char k[][4]);	// ����Կ��

	void invSubBytes(unsigned char state[][4]);							// ���ֽ����
	void invShiftRows(unsigned char state[][4]);						// ������λ
	void invMixColumns(unsigned char state[][4]);						// ���л���

private:
	unsigned char mSbox[256];
	unsigned char mInvSbox[256];
	unsigned char mW[11][4][4];
};

// ���ܽӿ�
void* aes_encrypt(void* data, unsigned long length, unsigned char* key);

// ���ܽӿ�
void* aes_decrypt(void* data, unsigned long length, unsigned char* key);

#endif	// _AES_H_