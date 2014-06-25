__kernel void Bilinear_beta(__global unsigned char *pSrc,  int width,  int height, __global  unsigned char *pDes, int t_width, int t_height)
{
	int i=get_global_id(0);
	int j=get_global_id(1);

	float zx = (float)(width) / (t_width);
	float zy = (float)(height) / (t_height);

	unsigned char d,rd, ld, lu, ru;
	float x,y,n,m;

	y=j*zy;
	x=i*zx;

	n=y - (int)(y);
	m=x - (int)(x);

	ld = *(pSrc + width * (1+(int)(y)) + (int)(x));//��xia�ǵ������ֵ
	rd = *(pSrc + width * (1+(int)(y)) + (int)(x) + 1);// ���Ͻǵ������ֵ
	lu = *(pSrc + width* (int)(y) + (int)(x));// ���½ǵ������ֵ
	ru = *(pSrc + width* (int)(y) + (int)(x) + 1);// ���Ͻǵ������ֵ

	d= (unsigned char)((1 - m) * (1 - n) * lu + (1 - m) * n * ld + m * n * rd + m * (1 - n) * ru);//˫���Բ�ֵ���õ��任�������
	*(pDes+j*t_width+i)=d;
}