
#define REMOTE_VAL_NUM  20
#define REMOTE_VAL_NAME_LEN    16

#define M2100_INIT_END_SEND 26
#define M2100_INIT_SETUP_IPNET_IPADDR_SEND 25
#define M2100_INIT_SETUP_IPNET_SOCKET_SEND 24
#define M2100_INIT_SETUP_IPNET_CMNET_SEND 23
#define M2100_INIT_SETUP_IPNET_GPRS0_SEND 22
#define M2100_INIT_SETUP_IPSIO_SEND 21
#define M2100_INIT_SETUP_CGATT_SEND 20
#define M2100_INIT_SETUP_CGACT_SEND 19
#define M2100_INIT_SETUP_CSQ_SEND 18
#define M2100_INIT_SETUP_SFUN_SEND 17
#define M2100_INIT_SETUP_CREG_SEND 16
#define M2100_INIT_SETUP_CGDCONT_SEND 15
#define M2100_INIT_START 14
//#define M2100_INIT_CHECK_COM   14  //检测串口通讯
#define M2100_INIT_SETUP_CGCLASS   13  //设置平台类别
#define M2100_INIT_SETUP_CGDCONT   12  //设置连接方式
#define M2100_INIT_SETUP_CREG   11   //查询网络状态
#define M2100_INIT_SETUP_SFUN   10  //设置工作模式为4
#define M2100_INIT_SETUP_CSQ   9  //查询信号质量
#define M2100_INIT_SETUP_CGACT   8  //激活PDP
#define M2100_INIT_SETUP_CGATT   7  //GPRS
#define M2100_INIT_SETUP_IPSIO   6  //INTERNET 服务模式
#define M2100_INIT_SETUP_IPNET_GPRS0   5  //建立连接模式
#define M2100_INIT_SETUP_IPNET_CMNET   4  //建立连接模式
#define M2100_INIT_SETUP_IPNET_SOCKET   3  //建立连接模式
#define M2100_INIT_SETUP_IPNET_IPADDR   2  //建立连接模式
#define M2100_INIT_END   1


#define M2100_HARD_RST_START  2 //低电平，维持1秒  
#define M2100_HARD_RST_END 1 //1-恢复高电平，上电复位结束

char M2110_MAT[]={"AT^IPSRV=0, \"address\", \"socktcp:/" "/183.230.40.33:80\"\r\n"};

#define RECORD_NUM  1000

struct REMOTE
{
 unsigned int addr; //数据源地址
 int change;  //0-不传送
 int dot; //小数点位置,>0为正数，<0有符号数
 char name[REMOTE_VAL_NAME_LEN];
}remote_para[REMOTE_VAL_NUM];

struct 
{
 unsigned int no; //0-65536
 unsigned int addr; //数据地址，0xffff,未格式化
 float val;  //0-不传送
 unsigned char time[6]; //年，月，日，时，分，秒
}record,record_last;


#define GPRS_IP_LEN  2
#define GPRS_PORT_LEN  1
#define GPRS_USERID_LEN  8
#define GPRS_USERKEY_LEN  25

#define GPRS_PARA_ADDR  300
#define GPRS_END_ADDR GPRS_PORT_ADDR+1
#define GPRS_USERID_ADDR  GPRS_PARA_ADDR+3
#define GPRS_USERKEY_ADDR  GPRS_USERID_ADDR+8
#define GPRS_PORT_ADDR  GPRS_USERKEY_ADDR+25

struct 
{
unsigned char srvip[GPRS_IP_LEN*2];
unsigned int  srvport;
char userid[GPRS_USERID_LEN*2];
char userkey[GPRS_USERKEY_LEN*2];
}gprs;

#define GPRS_PARA_LEN sizeof(gprs)/2

int gprs_datalen;
char gprs_val_name[REMOTE_VAL_NAME_LEN];
char gprs_val_time[22];
char gprs_val[16];


#define REMOTE_PARA_ADDR_START  400
#define REMOTE_PARA_LEN   11
#define REMOTE_PARA_ADDR_END  REMOTE_PARA_ADDR_START+REMOTE_PARA_LEN*REMOTE_VAL_NUM

#define RECORD_ADDR_START   1000
#define RECORD_LEN  7
#define RECORD_ADDR_END RECORD_ADDR_START+RECORD_LEN*RECORD_NUM

float remote_val[REMOTE_VAL_NUM];
float remote_val_last[REMOTE_VAL_NUM];
unsigned char remote_time[REMOTE_VAL_NUM][RTC_BUF_LEN];
int  remote_flag[REMOTE_VAL_NUM]; //0-不发送，1-准发送

#define GPRS_BUF_LEN  250
#define M2100_ERR  -1

int m2100_step=0,m2100_waittime=0,m2100_error=0,m2100_fun_step=0;

char gprs_tx_buf[GPRS_BUF_LEN];
char gprs_rx_buf[GPRS_BUF_LEN];

void gprs_send(char *ptx,int len)
{
  
 com1_tx_len=len;
 com1_ptx=(unsigned char*)ptx;
 com1_rp=0;
  while (!(UC1IFG&UCA1TXIFG)); 
  
  UCA1TXBUF=*com1_ptx;
  com1_ptx+=(len);
  UC1IE|=UCA1TXIE;
  
}

int gprs_rev(char *prx,int max_len)
{
int len,m=0;
if (com1_deal_flag>0)
{
	//for (m=0;m<max_len;m++)*(prx+m)=0;
        //memset(prx,0,max_len);
	m=0;
	if (com1_deal_flag)
	{
	 len=com1_rp;
	 if (len>=max_len)len=max_len;
	 for (m=0;m<len;m++)
	 	{
	 	 *prx=com1_buf[m];
		 prx++;
	 	}
         //com1_rp=0;
	}
        *prx='\0';
}
com1_deal_flag=0;
return m;
}

int gprs_search(char *psrc,int srclen,char *pstr,int len) // <0,无正确数据,
{
 int m,n,val;
 for (m=0;m<srclen;m++)
 	{
 	 if (*psrc==*pstr)
 	 	{
 	 	 val=m;
 	 	 for (n=0;n<len;n++)
 	 	 	{
 	 	 	 if (*(psrc+n)!=*(pstr+n)) val=-1;
 	 	 	}
		 if (val==m) {com1_rp=0;return m;}
 	 	}
	 psrc++;
 	}
 com1_rp=0;
 return -1; //
}

#define M2100_POWER_RST_START  5 //3-高电平，维持1秒  
#define M2100_POWER_RST_SETUP_LOW 4 //2-低电平，维持3秒，
#define M2100_POWER_RST_REV 3 //2-低电平，维持3秒，
#define M2100_POWER_RST_REV2 2 //2-低电平，维持3秒，
#define M2100_POWER_RST_END 1 //1-恢复高电平，上电复位结束

int m2100_pwr_rst() //返回下一次执行命令，执行完成后，返回0
{
  
  int len=0;
	 	if (m2100_fun_step>0)
	 		{
	 		 switch (m2100_fun_step)
	 		 	{
	 		 	 case M2100_POWER_RST_START: 
				 default:
				 	M2100_PWR_HIGH;//保持1秒
					m2100_waittime=1; 
					m2100_fun_step=M2100_POWER_RST_SETUP_LOW;
				 break;
	 		 	 case M2100_POWER_RST_SETUP_LOW: 
				 	if (m2100_waittime==0) //等待1秒后
				 		{
						M2100_PWR_LOW;
						m2100_waittime=2; 
						m2100_fun_step=M2100_POWER_RST_END;
				 		}
				 break;
	 		 	 case M2100_POWER_RST_END: 				 	
					if (m2100_waittime==0) //3秒等待结束
						{
						M2100_PWR_HIGH;
						m2100_waittime=20; //保持1秒
                                                //
						m2100_fun_step=M2100_POWER_RST_REV;
						}
				 break;
                                 
                                   
	 		 	 case M2100_POWER_RST_REV: 	
                                          //判断接收数据是否正常
                                          if (com1_deal_flag>0)
                                          {
                                           len=gprs_rev(gprs_rx_buf,GPRS_BUF_LEN);                                           
                                           if  (len>=0) 
                                           {
                                             len=gprs_search(gprs_rx_buf,len,"^SYSHUT\r\n",7);    
                                             if (len>=0)
                                             {m2100_fun_step=M2100_POWER_RST_REV2;
                                             m2100_waittime=20;
                                             }
                                          }
					if (m2100_waittime==0) //启动失败
						{
						m2100_fun_step=M2100_POWER_RST_START;
						}
				 break;
	 		 	 case M2100_POWER_RST_REV2:
                                          if (com1_deal_flag>0)
                                          {
                                           len=gprs_rev(gprs_rx_buf,GPRS_BUF_LEN);                                           
                                           if  (len>0) 
                                           {
                                             len=gprs_search(gprs_rx_buf,len,"+SIMIND: 1\r\n",10); 
                                             
                                             if (len>=0){m2100_fun_step=0;m2100_waittime=1;}
                                             else m2100_waittime=10;
                                           }    
                                          }
					if (m2100_waittime==0) //启动失败
						{
						m2100_fun_step=M2100_POWER_RST_START;
						} 	
				 break;
	 		 	}
	 		}
                        }
	return m2100_fun_step;
}

int m2100_hard_rst() //返回下一次执行命令，执行完成后，返回0
{
	 	if (m2100_fun_step>0)
	 		{
	 		 switch (m2100_fun_step)
	 		 	{
	 		 	 case M2100_HARD_RST_START: 
				 default:
				 	M2100_RST_LOW;//保持1秒
					m2100_waittime=1; 
					m2100_fun_step=M2100_HARD_RST_END;
				 break;
	 		 	 case M2100_HARD_RST_END: 				 	
					if (m2100_waittime==0) //3秒等待结束
						{
						M2100_RST_HIGH;
						m2100_waittime=0; //保持1秒
						m2100_fun_step=0;
						}
				 break;
	 		 	}
	 		}
	return m2100_fun_step;
}

#define M2100_SOFT_RST_START  2 //低电平，维持1秒  
#define M2100_SOFT_RST_END 1 //1-恢复高电平，上电复位结束

int m2100_soft_rst() //返回下一次执行命令，执行完成后，返回0
{
	 	if (m2100_fun_step>0)
	 		{
	 		 switch (m2100_fun_step)
	 		 	{
	 		 	 case M2100_HARD_RST_START: 
				 default:
				 //	COM2_M2100_RST_LOW;//保持1秒
					m2100_waittime=1; 
					m2100_fun_step=M2100_HARD_RST_END;
				 break;
	 		 	 case M2100_HARD_RST_END: 				 	
					if (m2100_waittime==0) //3秒等待结束
						{
					//	COM2_M2100_RST_HIGH;
						m2100_waittime=0; //保持1秒
						m2100_fun_step=0;
						}
				 break;
	 		 	}
	 		}
	return m2100_fun_step;
}

#define M2100_POWER_RST 0
#define M2100_HARD_RST    M2100_POWER_RST+1
#define M2100_SOFT_RST    M2100_HARD_RST+1
#define M2100_INIT        M2100_SOFT_RST+1
#define M2100_SETUP       M2100_INIT+1
#define M2100_CONNECT     M2100_SETUP+1
#define M2100_SEND        M2100_CONNECT+1
#define M2100_SLEEP       M2100_SEND+1
#define M2100_END         M2100_SLEEP+1

#define M2100_CONNECT_SWR_REV 5
#define M2100_CONNECT_START_REV 4
#define M2100_CONNECT_START     3
#define M2100_CONNECT_SWR       2
#define M2100_CONNECT_END     1
int m2100_connect()
	{ 
	 int len;
			if (m2100_fun_step>0)
				{
				 switch (m2100_fun_step)
					{
					 case M2100_CONNECT_START: 
					 default:
						gprs_send("AT^IPSTART=0\r\n",14);
						m2100_waittime=2; 
						m2100_fun_step=M2100_CONNECT_START_REV;
					 break;
                                        case M2100_CONNECT_START_REV:
                                                if (m2100_waittime==0)
						{
						 gprs_rev(gprs_rx_buf,GPRS_BUF_LEN);
						 len=gprs_search(gprs_rx_buf,com1_rp,"AT^IPSTART=0",12);
						 if (len<0) 
                                                 {
                                                   m2100_error++;	
                                                   m2100_fun_step=M2100_CONNECT_START;
                                                 }						   
						 else { 	
                                                        m2100_fun_step=M2100_CONNECT_SWR;
							m2100_error=0;
                                                 }
                                                }
                                          break;
					 case M2100_CONNECT_SWR:  //分析反馈数据
						gprs_send("AT^IPSWR=0,225\r\n",16);
						m2100_waittime=2; 
						m2100_fun_step=M2100_CONNECT_SWR_REV;
					 break;
					 case M2100_CONNECT_SWR_REV:  //分析反馈数据
						if (m2100_waittime==0)
						{
						 len=gprs_rev(gprs_rx_buf,GPRS_BUF_LEN);
						 len=gprs_search(gprs_rx_buf,len,",",1);
						 if (len<0) 
                                                 {
                                                   m2100_error++;
                                                   m2100_fun_step=M2100_CONNECT_SWR;
                                                 }
						 else { 					
							m2100_fun_step=0;
							m2100_error=0;
							m2100_waittime=0;
							}
						 }
					 break;
                                        
                                         case M2100_CONNECT_END:
                                           
                                           break;
					}
				}
	  return m2100_fun_step;
	}


#define M2100_SEND_START     2
#define M2100_SEND_END     1


#define M2100_SLEEP_START     3
#define M2100_SLEEP_CMD     2
#define M2100_SLEEP_END     1
int  m2100_sleep()  //休眠模式，未调试过
	{ 
		 int len;
				if (m2100_fun_step>0)
					{
					 switch (m2100_fun_step)
						{
						 case M2100_SLEEP_START: 
						 default:
							gprs_send("AT^IPSTART=0\r\n",14);
							m2100_waittime=1; 
							m2100_fun_step=M2100_CONNECT_SWR;
						 break;
						 case M2100_SLEEP_CMD:  //分析反馈数据
							if (m2100_waittime==0)
							{
							 len=gprs_rev(gprs_rx_buf,GPRS_BUF_LEN);
							 len=gprs_search(gprs_rx_buf,len,"OK\r\n",4);
							 if (len<0) m2100_error++;							   
							 else { 					
								gprs_send("AT^IPSWR=0,225\r\n",16);
								m2100_fun_step=M2100_CONNECT_END;
								m2100_error=0;
								m2100_waittime=1;
								}
							 }
						 break;
						 case M2100_SLEEP_END:  //分析反馈数据
							if (m2100_waittime==0)
							{
							 len=gprs_rev(gprs_rx_buf,GPRS_BUF_LEN);
							 len=gprs_search(gprs_rx_buf,len,"OK\r\n",4);
							 if (len<0) m2100_error++;							   
							 else { 					
								m2100_fun_step=0;
								m2100_error=0;
								m2100_waittime=0;
								}
							 }
						 break;
						}
					}
		  return 0;
		}



int remote_get_val(unsigned int addr) //需要发送数据 0-不发放，1-发送
{
 unsigned int val_addr,m;
 int val_change,val_dot,val;
 float f1,f2;
 int flag=0;
 if (addr>=REMOTE_VAL_NUM) return 0;
 else
 	{
 	 val_addr=remote_para[addr].addr;
	 val_change=remote_para[addr].change;
	 val_dot=remote_para[addr].dot;	 
 	}
 if (val_addr>=50000) //5区，寄存器输出
 	{
 	f1=0;f2=0;
 	}
 else if (val_addr>=40000) //4区，寄存器输出
 	{
 	 val_addr-=40000;
	 if (val_dot>0) {f1=val_rd_reg(val_addr);m=val_dot;}
	 else {val=(int)val_rd_reg(val_addr);f1=val;m=0-val_dot;}
	 f2=val_change;
	 if (f2<0) f2=0-f2;
	 for (;m>0;m--) {f1/=10;f2/=10;}
 	}
 else if (addr>=30000) //3区，模拟量输入
 	{
 	 val_addr-=30000;
	 if (val_dot>=0) {f1=val_rd_ai(val_addr);m=val_dot;}
	 else {val=(int)val_rd_ai(val_addr);f1=val;m=0-val_dot;}
	 f2=val_change;
	 if (f2<0) f2=0-f2;
	 for (;m>0;m--) {f1/=10;f2/=10;}
 	}
 else if (addr>=10000) //1区，开关量输入
 	{
 	 val_addr-=10000;
	 f1=val_rd_di(val_addr);
	 f2=val_change;
 	}
 else //0区，继电器输出
 	{
	 f1=val_rd_do(val_addr);
	 f2=val_change;
 	}
 remote_val[addr]=f1;
 if (f2==0) flag=0;
 else if ((f1+f2>=(remote_val_last[addr]))||(f1-f2<=(remote_val_last[addr])))flag=1;
 else flag=0;
 return flag;
}
unsigned int remote_val_addr=0;



unsigned record_wr_addr,record_rd_addr,record_no=0;
int record_write(unsigned int addr) //0-写成功，-1-失败
{
 unsigned char i,*p;
 unsigned int *pint,m,val,n;
 if (addr>=RECORD_NUM) return 1;
 else 
 	{
 	 record.addr=addr;
	 record.val=remote_val_last[addr];
	 p=&remote_time[0][0];
	 p+=addr*6;
	 for (i=0;i<6;i++)record.time[i]=*p++;
	 if (record_no==0) record_no=1;
	 record.no=record_no++;
	 pint=&record.no;
	 for (n=0;n<RECORD_NUM;n++)
	 	{
		 if (record_wr_addr>=RECORD_NUM)record_wr_addr=0;
		 m=record_wr_addr*RECORD_LEN+RECORD_ADDR_START;
		 //检查是否有数据
		 val=eep_rd_word(m);
	 	 if (val==0) //无数据，可以写入
	 	 	{
	 	 	 if (record_wr_addr==(RECORD_NUM-1))eep_wr_word(RECORD_ADDR_START,0);
	 	 	 else eep_wr_word(m+RECORD_LEN,0); //清除下一条历史记录
	 	 	 for (i=0;i<RECORD_LEN;i++) eep_wr_word(m++,*pint++);
		 	 record_wr_addr++;
			 return 0;
	 	 	}
		 else record_wr_addr++;
	 	}
 	}
 return 1;
}

unsigned int record_wr_init()//查找当前历史记录的位置
{
 unsigned int m,n,val;
	 for (n=0;n<RECORD_NUM;n++)
	 	{
		 m=n*RECORD_LEN+RECORD_ADDR_START;
		 val=eep_rd_word(m);
	 	 if (val==0) return n;
	 	}
	 return 0;
}
unsigned int record_rd(unsigned int *pval) //0-没有历史记录了
{
 unsigned int m,n,val,addr;
 
	 for (n=0;n<RECORD_NUM;n++)
	 	{
	 	 if (record_rd_addr>=RECORD_NUM)record_rd_addr=0;
		 addr=record_rd_addr*RECORD_LEN+RECORD_ADDR_START;
		 val=eep_rd_word(addr);
	 	 if (val<REMOTE_VAL_NUM)
	 	 	{
	 	 	 *pval++=val;
	 	 	 for (m=1;m<RECORD_LEN;m++)*pval++=eep_rd_word(addr+m);			 
	 	 	}
		 else record_rd_addr++;
	 	}
	 if (n==RECORD_NUM) return 0;
	 else return 1; 
}

unsigned int m2100_send_data_flag=0; //1-发送实时数据，2-发送历史数据
char JSON[]="POST /devices/3031947/datapoints HTTP/1.1\r\napi-key: Wxkw6jQFx0EjQC8vOYbjzX8ZCec=\r\nHost: api.heclouds.com\r\nContent-Length: 61\r\n\r\n{\"datastreams\":[{\"id\":\"temp\",\"datapoints\":[{\"value\":\"11\"}]}]}\r\n\r\n";

int m2100_send() //0-无数据发送，
{ 
 int len,m,n;
	 	if (m2100_fun_step>0)
	 	{
	 		 switch (m2100_fun_step)
	 		 {
	 		 	 case M2100_SEND_START: 
				 //default:
				 	// 准备发送要发送的数据
				 	if (m2100_waittime==0)
                                        {
				 	        if (m2100_send_data_flag>0) //重发上次的数据
				 		{
                                                    gprs_send(gprs_tx_buf,strlen(gprs_tx_buf));	
                                                    m2100_waittime=5; 
                                                    m2100_fun_step=M2100_SEND_END;
				 		}
					        else
						{
                                                    /*memset(gprs_val_name,0,sizeof(gprs_val_name));
                                                    memset(gprs_val_time,0,sizeof(gprs_val_time));
                                                    memset(gprs_val,0,sizeof(gprs_val));
                                                    memset(gprs_tx_buf,0,sizeof(gprs_tx_buf));
                                                    for (m=0;m<REMOTE_VAL_NUM;m++)
                                                    {
                                                         if (remote_val_addr>=REMOTE_VAL_NUM)remote_val_addr=0;
                                                         if (remote_flag[remote_val_addr]>0) //有数据需要发送
                                                         {
                                                                 for (n=0;n<REMOTE_VAL_NAME_LEN;n++)
                                                                 {
                                                                         if (remote_para[remote_val_addr].name[n]!=0) gprs_val_name[n]=remote_para[remote_val_addr].name[n];
                                                                 }
                                                                 sprintf(gprs_val_time,"20%2d-%2d-%2dT%2d:%2d:%2d",rtctime.year,rtctime.month,rtctime.day,rtctime.hour,rtctime.minute,rtctime.second);
                                                                 sprintf(gprs_val,"%f",remote_val[remote_val_addr]);
                                                                                                                                                
                                                                 gprs_datalen=82+strlen(gprs_val_name)+strlen(gprs_val_time),strlen(gprs_val);
                                                                 m2100_send_data_flag=2;
                                                          }
                                                          else remote_val_addr++;
                                                          if (m2100_send_data_flag>0) break;
                                                    }
                                                    if (m2100_send_data_flag==0) //当前无新数据，查看是否需要发送历史数据
                                                    {
                                                             m2100_send_data_flag=record_rd(&record_last.addr);
                                                             //m2100_send_data_flag=record_rd(record_last.addr);
                                                             if (m2100_send_data_flag>0)
                                                             {						 	
                                                                    for (n=0;n<REMOTE_VAL_NAME_LEN;n++)
                                                                    {
                                                                             if (remote_para[record_last.addr].name[n]!=0) gprs_val_name[n]=remote_para[record_last.addr].name[n];
                                                                    }
                                                                    sprintf(gprs_val_time,"20%2d-%2d-%2dT%2d:%2d:%2d",record_last.time[0],record_last.time[1],record_last.time[2],record_last.time[3],record_last.time[4],record_last.time[5]);
                                                                    sprintf(gprs_val,"%f",record_last.val);
                                                                    gprs_datalen=82+strlen(gprs_val_name)+strlen(gprs_val_time),strlen(gprs_val);
							 	}
						    }
						    if (m2100_send_data_flag>0)
						    {						
                                                          sprintf(gprs_tx_buf,"POST /devices/%s/datapoints HTTP/1.1\r\napi-key:%s\r\nHost:api.heclouds.com\r\nContent-Length:%d\r\n\r\n{\"datastreams\":[{\"id\":\"%s\",\"dataponints\":[{\"at\":\"%s\",\"value\":\"%s\"}]}]}]\r\n\r\n",gprs.userid,gprs.userkey,gprs_datalen,gprs_val_name,gprs_val_time,gprs_val);
                                                          gprs_send(gprs_tx_buf,strlen(gprs_tx_buf));	
                                                          m2100_waittime=1; 
                                                          m2100_fun_step=M2100_SEND_END;
						    }
						    else return 0;*/ //无新数据发送
                                                  //test OneNet
                                                  com1_rp=0;
                                                  m2100_fun_step=M2100_SEND_END;
                                                  m2100_waittime=3;
                                                  gprs_send(JSON,193);	
                                                     
                                                    
						}
				 	}
				 break;
	 		 	 case M2100_SEND_END:  //分析反馈数据
                                        if (m2100_waittime==0)
                                        {
                                             len=gprs_rev(gprs_rx_buf,GPRS_BUF_LEN);
                                             len=gprs_search(gprs_rx_buf,com1_rp,"OK\r\n",4);
                                             if (len<0) m2100_error++;                             
                                             else {	
					 	        switch(m2100_send_data_flag)
					 		{ 
					 		 case 2:							 	
								remote_val_addr++;
								remote_flag[remote_val_addr]=0;
							 break;
							 case 1:
								eep_wr_word(record_rd_addr*RECORD_LEN+RECORD_ADDR_START,0);
								record_rd_addr++;
							 break;
							 default:
							 	//m2100_send_data_flag=0;
							 break;
					 		}
                                                        m2100_send_data_flag=0;							
                                                        m2100_fun_step=M2100_SEND_START;
                                                        m2100_error=0;
                                                        //m2100_waittime=1;
						}
                                       }
				 break;
	 		 }
	 	}
  return 0;
}

int m2100_end()
{
  return 0;
}

int m2100_init() //返回下一次执行命令，执行完成后，返回0
{
 int len;
	 	if (m2100_fun_step>0)
	 	{
	 		 switch (m2100_fun_step)
	 		 {
				 default:
				// 	gprs_send("AT\r\n",4);
                   //                gprs_send("AT+CGCLASS=\"B\"\r\n",16);
				//	m2100_waittime=3; 
				//	m2100_fun_step=M2100_INIT_CHECK_COM;
				 break;
	 		 	 case M2100_INIT_START: 
				 		gprs_send("AT+CGCLASS=\"B\"\r\n",16);
					 	m2100_fun_step=M2100_INIT_SETUP_CGCLASS;
						//m2100_error=0;
						m2100_waittime=1;
				 break;
				// #define M2100_INIT_SETUP_CGCLASS   13  //设置平台类别
	 		 	 case M2100_INIT_SETUP_CGCLASS:  //分析反馈数据
                                        if (m2100_waittime==0)
                                        {
                                           len=gprs_rev(gprs_rx_buf,GPRS_BUF_LEN);
                                           len=gprs_search(gprs_rx_buf,com1_rp,"OK\r\n",4);
                                           if (len<0)
                                           {
                                             m2100_fun_step=M2100_INIT_START;   
                                             m2100_error++;      
                                           }
                                            else 	
                                            {
                                                  m2100_fun_step=M2100_INIT_SETUP_CGDCONT_SEND;
						  m2100_error=0;
                                            }
                                        }
				 break;
				 //#define M2100_INIT_SETUP_CGDCONT   12  //设置连接方式
                                case M2100_INIT_SETUP_CGDCONT_SEND:
                                                gprs_send("AT+CGDCONT=1,\"IP\",\"CMNET\"\r\n",27);
					 	m2100_fun_step=M2100_INIT_SETUP_CGDCONT;
                                                //m2100_error=0;
						m2100_waittime=1;
                                  break;
	 		 	 case M2100_INIT_SETUP_CGDCONT:  //分析反馈数据
                                        if (m2100_waittime==0)
                                        {
                                         gprs_rev(gprs_rx_buf,GPRS_BUF_LEN);
					 len=gprs_search(gprs_rx_buf,com1_rp,"OK\r\n",4);
                                        if (len<0) 
                                        {
                                         m2100_error++;     
                                         m2100_fun_step=M2100_INIT_SETUP_CGDCONT_SEND;
                                        }
                                        else {					 	
				 		
						m2100_error=0;
						m2100_fun_step=M2100_INIT_SETUP_CREG_SEND;
						}
                     }
				 break;	 		
                                 case M2100_INIT_SETUP_CREG_SEND:
                                                gprs_send("AT+CREG=1\r\n",11);
					 	m2100_fun_step=M2100_INIT_SETUP_CREG;
                                                m2100_waittime=1;
                                    break;
				 //#define M2100_INIT_SETUP_CREG   11   //查询网络状态
				 case M2100_INIT_SETUP_CREG:  //分析反馈数据
                                        if (m2100_waittime==0)
                                        {
                                                 gprs_rev(gprs_rx_buf,GPRS_BUF_LEN);
                                                 len=gprs_search(gprs_rx_buf,com1_rp,"OK\r\n",4);
                                         if (len<0) 
                                         {
                                           m2100_error++;
                                           m2100_fun_step=M2100_INIT_SETUP_CREG_SEND;    
                                         }            
                                         else {					 	
				 		m2100_fun_step=M2100_INIT_SETUP_SFUN_SEND;
						m2100_error=0;
						}
                     }
				 break;	 
                                 case M2100_INIT_SETUP_SFUN_SEND:
                                                gprs_send("AT+SFUN=4\r\n",11);
					 	m2100_fun_step=M2100_INIT_SETUP_SFUN;
						m2100_waittime=1;
                                   break;
				 //#define M2100_INIT_SETUP_SFUN   10  //设置工作模式为4
				 case M2100_INIT_SETUP_SFUN:  //分析反馈数据
                                          if (m2100_waittime==0)
                                          {
                                             gprs_rev(gprs_rx_buf,GPRS_BUF_LEN);
                                             len=gprs_search(gprs_rx_buf,com1_rp,"OK\r\n",4);
                                             if (len<0) 
                                             {
                                               m2100_error++;  
                                               m2100_fun_step=M2100_INIT_SETUP_SFUN_SEND;
                                             }
                                             else {	
					 	m2100_fun_step=M2100_INIT_SETUP_CSQ_SEND;
						m2100_error=0;
						m2100_waittime=1;
						}
                                          }
				 break;
                                 case M2100_INIT_SETUP_CSQ_SEND:
                                   if(m2100_waittime==0)
                                   {
                                          gprs_send("AT+CSQ\r\n",11);
                                          m2100_fun_step=M2100_INIT_SETUP_CSQ;
                                          m2100_waittime=1;
                                   }
                                   break;
//#define M2100_INIT_SETUP_CSQ   9  //查询信号质量
				 case M2100_INIT_SETUP_CSQ:  
                                        if (m2100_waittime==0)
                                        {
                                             gprs_rev(gprs_rx_buf,GPRS_BUF_LEN);
                                             len=gprs_search(gprs_rx_buf,com1_rp,"OK\r\n",4);
                                             if (len<0)
                                             {
                                               m2100_error++; 
                                               m2100_fun_step=M2100_INIT_SETUP_CSQ_SEND;
                                               break;
                                             }
                                             if(len==27)
                                             {
                                               len=gprs_search(gprs_rx_buf,31,"1,99\r\n",4);
                                               if(len>0)
                                               {
                                                 m2100_error++; 
                                                 m2100_fun_step=M2100_INIT_SETUP_SFUN_SEND;
                                                 break;
                                               }
                                             }				 	
                                             m2100_fun_step=M2100_INIT_SETUP_CGACT_SEND;
                                             //m2100_fun_step=M2100_INIT_SETUP_IPSIO_SEND;
                                             m2100_error=0;
                                                    
                                        }
				 break;
                                 case M2100_INIT_SETUP_CGACT_SEND:
                                                gprs_send("AT+CGACT=1,1\r\n",14);
					 	m2100_fun_step=M2100_INIT_SETUP_CGACT;
						m2100_waittime=1;
                                   break;
//#define M2100_INIT_SETUP_CGACT   8  //激活PDP
				 case M2100_INIT_SETUP_CGACT:  
                                        if (m2100_waittime==0)
                                        {
                                             gprs_rev(gprs_rx_buf,GPRS_BUF_LEN);
                                             len=gprs_search(gprs_rx_buf,com1_rp,"OK\r\n",4);
                                             if (len<0) 
                                             {
                                               m2100_error++;
                                               m2100_fun_step=M2100_INIT_SETUP_CGACT_SEND;
                                             }
                                             else {					 	
                                                     m2100_fun_step=M2100_INIT_SETUP_CGATT_SEND;
                                                     m2100_error=0;   
                                                  }
                                         }
				 break;
                                
                                   case M2100_INIT_SETUP_CGATT_SEND:
                                                gprs_send("AT+CGATT=1\r\n",12);
                                                m2100_fun_step=M2100_INIT_SETUP_CGATT;
                                                m2100_waittime=1;
                                     break;
//#define M2100_INIT_SETUP_CGATT   7  //GPRS
				 case M2100_INIT_SETUP_CGATT:  
                                        if (m2100_waittime==0)
                                        {
                                           gprs_rev(gprs_rx_buf,GPRS_BUF_LEN);
                                           len=gprs_search(gprs_rx_buf,com1_rp,"OK\r\n",4);
                                           if (len<0) 
                                           {
                                             m2100_error++; 
                                             m2100_fun_step=M2100_INIT_SETUP_CGATT_SEND;
                                           }
                                           else {				
					 	m2100_fun_step=M2100_INIT_SETUP_IPSIO_SEND;
						m2100_error=0;
						}
                                        }
				 break;
                                 case M2100_INIT_SETUP_IPSIO_SEND:
                                                gprs_send("AT^IPSIO=0,1\r\n",14);
					 	m2100_fun_step=M2100_INIT_SETUP_IPSIO;
						m2100_waittime=1;
                                   break;
//#define M2100_INIT_SETUP_IPSIO   6  //INTERNET 服务模式
				 case M2100_INIT_SETUP_IPSIO:  
                                          if (m2100_waittime==0)
                                          {
                                           gprs_rev(gprs_rx_buf,GPRS_BUF_LEN);
                                           len=gprs_search(gprs_rx_buf,com1_rp,"OK\r\n",4);
                                           if (len<0) 
                                           {
                                             m2100_error++;  
                                             m2100_fun_step=M2100_INIT_SETUP_IPSIO_SEND;
                                           }
                                           else {	
					 	m2100_fun_step=M2100_INIT_SETUP_IPNET_GPRS0_SEND;
						m2100_error=0;
						}
                                           }
				 break;
                                 case M2100_INIT_SETUP_IPNET_GPRS0_SEND:
                                                gprs_send("AT^IPNET=0, \"conType\", \"GPRS0\"\r\n",32);
					 	m2100_fun_step=M2100_INIT_SETUP_IPNET_GPRS0;
						m2100_waittime=1;
                                   break;
//#define M2100_INIT_SETUP_IPNET_GPRS0   5  //建立连接模式
				 case M2100_INIT_SETUP_IPNET_GPRS0:  
                                        if (m2100_waittime==0)
                                        {
                                           gprs_rev(gprs_rx_buf,GPRS_BUF_LEN);
                                           len=gprs_search(gprs_rx_buf,com1_rp,"OK\r\n",4);
                                           if (len<0) 
                                           {
                                             m2100_error++; 
                                             m2100_fun_step=M2100_INIT_SETUP_IPNET_GPRS0_SEND;
                                           }
                                           else {					 	
					 	m2100_fun_step=M2100_INIT_SETUP_IPNET_CMNET_SEND;
						m2100_error=0;
						}
                                        }
				 break;
                                 case M2100_INIT_SETUP_IPNET_CMNET_SEND: 
                                                gprs_send("AT^IPNET=0, \"apn\", \"cmnet\"\r\n",28);
					 	m2100_fun_step=M2100_INIT_SETUP_IPNET_CMNET;
						m2100_waittime=1;
                                   break;
//#define M2100_INIT_SETUP_IPNET_CMNET   4  //建立连接模式
				 case M2100_INIT_SETUP_IPNET_CMNET:  
                                          if (m2100_waittime==0)
                                          {
                                             gprs_rev(gprs_rx_buf,GPRS_BUF_LEN);
                                             len=gprs_search(gprs_rx_buf,com1_rp,"OK\r\n",4);
                                             if (len<0) 
                                             {
                                               m2100_error++;  
                                               m2100_fun_step=M2100_INIT_SETUP_IPNET_CMNET_SEND;
                                             }
                                             else {					 	
					 	m2100_fun_step=M2100_INIT_SETUP_IPNET_SOCKET_SEND;
						m2100_error=0;
						}
                                         }
				 break;
                                 case M2100_INIT_SETUP_IPNET_SOCKET_SEND:
                                                gprs_send("AT^IPSRV=0, \"srvType\", \"socket\"\r\n",33);
					 	m2100_fun_step=M2100_INIT_SETUP_IPNET_SOCKET;
						m2100_waittime=1;
                                   break;
//#define M2100_INIT_SETUP_IPNET_SOCKET   3  //建立连接模式
				 case M2100_INIT_SETUP_IPNET_SOCKET:  
                                          if (m2100_waittime==0)
                                          {
                                             gprs_rev(gprs_rx_buf,GPRS_BUF_LEN);
                                             len=gprs_search(gprs_rx_buf,com1_rp,"OK\r\n",4);
                                             if (len<0) 
                                             {
                                               m2100_error++;   
                                               m2100_fun_step=M2100_INIT_SETUP_IPNET_SOCKET_SEND;
                                             }
                                             else {					
					 	m2100_fun_step=M2100_INIT_SETUP_IPNET_IPADDR_SEND;
						m2100_error=0;
						}
                                          }
				 break;
                                 case M2100_INIT_SETUP_IPNET_IPADDR_SEND:
                                                
                                                gprs_send(M2110_MAT,53);
					 	m2100_fun_step=M2100_INIT_SETUP_IPNET_IPADDR;
						m2100_waittime=1;
                                   break;

//#define M2100_INIT_SETUP_IPNET_IPADDR   2  //建立连接模式

				 case M2100_INIT_SETUP_IPNET_IPADDR:  
                                          if (m2100_waittime==0)
                                          {
                                             gprs_rev(gprs_rx_buf,GPRS_BUF_LEN);
                                             len=gprs_search(gprs_rx_buf,com1_rp,"OK\r\n",4);
                                             if (len<0) 
                                             {
                                               m2100_error++;   
                                               m2100_fun_step=M2100_INIT_SETUP_IPNET_IPADDR_SEND;
                                             }
                                             else {			
					 	m2100_fun_step=M2100_HARD_RST_END;
						m2100_error=0;
						m2100_waittime=0;
						}
                                            }
				 break;
                                
	 		 	}
	 		}
	return m2100_fun_step;
}
//定时更新时间
unsigned int remote_update_time=0; //分钟为单位，默认时间为10分钟 600 

void remote_logic()
{
 unsigned int m,n,flag;
 unsigned char *p;
	//判断是否有新数据产生
	p=&rtctime.year;
	for (m=0;m<REMOTE_VAL_NUM;m++)
	   {
	   flag=remote_get_val(m); 
	   if (flag>0)
		   {
		   if (remote_flag[m]!=0)record_write(m);//存储历史数据 
		   for (n=0;n<RTC_BUF_LEN;n++) remote_time[m][n]=*p++;
		   remote_val_last[m]=remote_val[m];
		   remote_flag[m]=1;
		   }
	   }
	
	//是否达到定时传送条件
	if (remote_update_time>dev.remote_time) //设置的参数中，变化设置为0的数据不传送
	   {
		for (m=0;m<REMOTE_VAL_NUM;m++)
		   {
			if (remote_para[m].change!=0)
			   {
		       if (remote_flag[m]!=0)record_write(m);//存储历史数据 
			   remote_val_last[m]=remote_val[m];
		   	   for (n=0;n<RTC_BUF_LEN;n++) remote_time[m][n]=*p++;			   
			   remote_flag[m]=2;
			   remote_update_time=1; //重新计时
			   }
		   }
	   }
}
#define ERROR_MAX  100

void run_m2100()
{
 remote_logic(); 
 switch (m2100_step) 
 	{
 	 case M2100_POWER_RST:  //上电复位流程
	 	 if (m2100_pwr_rst()==0)
	 	 	{
	 	 	 m2100_step=M2100_INIT;
			 m2100_fun_step=M2100_INIT_START;
	 	 	}
          else if (m2100_error>ERROR_MAX)
          {
           m2100_step=M2100_POWER_RST;
           m2100_fun_step=M2100_POWER_RST_START;
          }
	 break;
 	 case M2100_HARD_RST:  //硬件复位流程
 	 if (m2100_hard_rst()==0)
 	 	{
 	 	 m2100_step=M2100_INIT;
		 m2100_fun_step=M2100_INIT_START;
 	 	}
	 break;
 	 case M2100_SOFT_RST:  //硬件复位流程
 	 if (m2100_soft_rst()==0)
 	 	{
 	 	 m2100_step=M2100_INIT;
		 m2100_fun_step=M2100_INIT_START;
 	 	}
	 break;
 	 case M2100_INIT:  //硬件复位流程
         if (M2100_VIO_RD==0) //关机状态
         {
           m2100_step=M2100_POWER_RST;
           m2100_fun_step=M2100_POWER_RST_START;
         }
         else  //发送测试命令，查看通讯是否正常
         {
           if (m2100_init()==1)
           {
             m2100_step=M2100_CONNECT;
             m2100_fun_step=M2100_CONNECT_START;
	   }
           else if (m2100_error>ERROR_MAX)
           {
             m2100_step=M2100_HARD_RST;
             m2100_fun_step=M2100_HARD_RST_START;
             m2100_error=0;
           }
         }
 	 
	 break;
 	 case M2100_CONNECT:  //硬件复位流程
 	 if (m2100_connect()==0)
 	 	{
                m2100_step=M2100_SEND;
                m2100_fun_step=M2100_SEND_START;
 	 	}
         else if(m2100_error>ERROR_MAX)
         {
           m2100_step=M2100_CONNECT;
           m2100_fun_step=M2100_SEND_START;
         }
           else if (m2100_error>ERROR_MAX)
           {
             m2100_step=M2100_HARD_RST;
             m2100_fun_step=M2100_HARD_RST_START;
           }
	 break;
 	 case M2100_SEND:  
 	 if (m2100_send()==0) //0-无数据发送
 	 	{         
 	 	 m2100_step=M2100_SLEEP;
         m2100_fun_step=M2100_SLEEP_START;
 	 	}
           else if (m2100_error>ERROR_MAX)
           {
             m2100_step=M2100_CONNECT;
             m2100_fun_step=M2100_CONNECT_START;
           }
	 break;
	 
 	 case M2100_SLEEP:  //硬件复位流程
 	// if (m2100_send()==0)
 	 	{
                 //检查是否发送完成所有数据
 	 	 m2100_step=M2100_CONNECT;
 	 	}
        //   else if (m2100_error>3)
           {
             m2100_step=M2100_CONNECT;
             m2100_fun_step=M2100_CONNECT_START;
           }
	 break;
	 
	 //#define M2100_END		 M2100_SLEEP+1
	 
	  case M2100_END:  //硬件复位流程
	  if (m2100_end()==0)
		 {
				  //检查是否发送完成所有数据
		  m2100_step=M2100_CONNECT;
		 }
		 //   else if (m2100_error>3)
			{
			  m2100_step=M2100_CONNECT;
			  m2100_fun_step=M2100_CONNECT_START;
			}
	  break;
	  

	 default:
	 	m2100_step=M2100_INIT;
	 break;
 	}
}

void init_m2100()
{
 unsigned int m,*pval;
 M2100_PWR_HIGH;
 M2100_RST_HIGH;
 M2100_PWR_INIT;
 M2100_RST_INIT;
 M2100_VIO_INIT;
 m2100_step=0;
 m2100_waittime=0;
 m2100_error=0;
 //m2100_step=M2100_INIT; 
 //m2100_fun_step=M2100_INIT_START;

           m2100_step=M2100_POWER_RST;
           m2100_fun_step=M2100_POWER_RST_START;
 

//    gprs_srvport=80;
  
//  gprs_userkey[50];
 pval=(unsigned int *)&gprs.srvip[0];
 *pval=eep_rd_word(GPRS_PARA_ADDR);
 if (*pval==0)
 	{
 	 gprs.srvip[0]=183;
 	 gprs.srvip[1]=230;
 	 gprs.srvip[2]=40;
 	 gprs.srvip[3]=33;
	 gprs.srvport=80;
  	 strcat(&gprs.userid[0],"3031947"); 
 	 strcat(&gprs.userkey[0],"Wxkw6jQFx0EjQC8vOYbjzX8ZCec="); 
	 pval=(unsigned int *)&gprs.srvip[0];
	 for (m=0;m<GPRS_PARA_LEN;m++) eep_wr_word(m+GPRS_PARA_ADDR,*pval++);
 	}
 else 
 	{
 	 for (m=0;m<GPRS_PARA_LEN;m++) *pval++=eep_rd_word(m+GPRS_PARA_ADDR);
 	}
 strcat(gprs_val_name,"test");
 strcat(gprs_val_time,"2016-01-01T12:01:01"); 
 strcat(gprs_val,"0.0");
 gprs_datalen=82+strlen(gprs_val_name)+strlen(gprs_val_time),strlen(gprs_val);
 
 pval=&remote_para[0].addr;
 m=eep_rd_word(REMOTE_PARA_ADDR_START);
 if (m>=REMOTE_VAL_NUM) //调用默认参数
 {
   for (m=0;m<REMOTE_VAL_NUM*REMOTE_PARA_LEN;m++) eep_wr_word(REMOTE_PARA_ADDR_START+m,0);
 }
 else for (m=0;m<REMOTE_VAL_NUM*REMOTE_PARA_LEN;m++) *pval++=eep_rd_word(m+REMOTE_PARA_ADDR_START); 
 
 for (m=0;m<REMOTE_VAL_NUM;m++) remote_flag[m]=0;
 
 remote_update_time=dev.remote_time;
 record_rd_addr=0;
 record_wr_addr=record_wr_init();
 
}

