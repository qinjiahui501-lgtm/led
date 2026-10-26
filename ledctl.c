#include "stdio.h"
#include "fcntl.h"
#include "errno.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "unistd.h"
#include "string.h"
#include "stdlib.h"

//gpio_dir是方向寄存器。gpio_reg是输出寄存器

int main(int argc,char const*argv[])
{
    if(argc!=2&&argc!=4&&argc!=5&&argc!=3)
    {
        printf("指令输入错误，格式为：./ledctl <命令> [参数]\n");
        printf("init\n");
        printf("set <编号0~7> <0/1>\n");
        printf("show\n");
        printf("status\n");
        printf("run <轮数>\n");
        printf("blink <编号> <次数>");
        printf("raw <8位二进制>\n");
        return -1;
    }
    if(strcmp(argv[1],"init")==0)
    {
        if(argc!=2)
        {
            printf("参数错误,请输入./ledctl init\n");
            return -1;
        }
        int fd=open("gpio_dir.dat",O_RDWR|O_CREAT|O_TRUNC,0644);
        if(fd<0)
        {
            printf("写入gpio_dir.dat失败,错误原因：%s,错误位置：%d行\n",strerror(errno),__LINE__);
            return -1;
        }
        write(fd,"11111111",8);
        int fc=open("gpio_reg.dat",O_RDWR|O_CREAT|O_TRUNC,0644);
        if(fc<0)
        {
            printf("写入gpio_reg.dat失败,错误原因：%s,错误位置：%d行\n",strerror(errno),__LINE__);
            return -1;
        }
        write(fc,"00000000",8);
        close(fd);
        close(fc);
    }
    else if(strcmp(argv[1],"set")==0)
    {
        if(argc!=4)
        {
            printf("参数错误,请输入./ledctl set <编号0~7> <0/1>\n");
            return -1;
        }
        int fc=open("gpio_reg.dat",O_RDONLY);
        if(fc<0)
        {
            printf("打开gpio_reg.dat失败,错误原因：%s,错误位置：%d行\n",strerror(errno),__LINE__);
            return -1;
        }
        char buf[9]={0};
        read(fc,buf,8);
        close(fc);
        char *endptr;//接收转换停止位置
        int reg=0,ID=strtol(argv[2],&endptr,10);
        if(*endptr!='\0'||endptr==argv[2])
        {
            printf("<编号0~7>输入错误,不是合法的纯数字\n");
            return -1;
        }
        if(ID>7||ID<0)
        {
            printf("请输入0~7其中一个数字\n");
            return -1;
        }
        int a=strtol(argv[3],&endptr,10);
        if(*endptr!='\0'||endptr==argv[3])
        {
            printf("<0/1>输入错误,不是合法的纯数字\n");
            return -1;
        }
        if(a>1||a<0)
        {
            printf("请输入0/1\n");
            return -1;
        }
        for(int i=0;i<8;i++)
        {
            reg=(reg<<1)|(buf[i]-'0');   
        }
        if(a==1)
        {
            reg |=(1<<ID);
        }
        else
        {
            reg &=~(1<<ID);
        }
        for(int i=0;i<8;i++)
        {
            buf[i]=(char)('0'+((reg>>(7-i))&1));
        }
        fc=open("gpio_reg.dat",O_WRONLY|O_TRUNC);
        if(fc<0)
        {
            printf("打开gpio_reg.dat失败,错误原因：%s,错误位置：%d行\n",strerror(errno),__LINE__);
            return -1;
        }
        write(fc,buf,8);
        close(fc);
    }
    else if(strcmp(argv[1],"show")==0)
    {
        if(argc!=2)
        {
            printf("参数错误,请输入./ledctl show\n");
            return -1;
        }
        static const char*const Bulb[2]={"[ ]","[■]"};
        char buf[9]={0};
        int fc=open("gpio_reg.dat",O_RDONLY),reg=0;
        if(fc<0)
        {
            printf("打开gpio_reg.dat失败,错误原因：%s,错误位置：%d行\n",strerror(errno),__LINE__);
            return -1;
        }
        read(fc,buf,8);
        close(fc);
        for(int i=0;i<8;i++)
        {
            reg=(reg<<1)|(buf[i]-'0');   
        }
        for(int i=7;i>=0;i--)
        {
            printf("%s",Bulb[(reg>>i)&1]);
        }
        printf("(0x%02X)\n",reg);
    }
    else if(strcmp(argv[1],"status")==0)
    {
        if(argc!=2)
        {
            printf("参数错误,请输入./ledctl status\n");
            return -1;
        }
        char buf[9]={0};
        int fd=open("gpio_dir.dat",O_RDONLY);
        if(fd<0)
        {
            printf("打开gpio_dir.dat失败,错误原因：%s,错误位置：%d行\n",strerror(errno),__LINE__);
            return -1;
        }
        read(fd,buf,8);
        close(fd);
        printf("dir=%s\n",buf);
        int fc=open("gpio_reg.dat",O_RDONLY);
        if(fc<0)
        {
            printf("打开gpio_reg.dat失败,错误原因：%s,错误位置：%d行\n",strerror(errno),__LINE__);
            return -1;
        }
        read(fc,buf,8);
        close(fc);
        printf("reg=%s\n",buf);
    }
    else if(strcmp(argv[1],"run")==0)
    {
        if(argc!=3)
        {
            printf("参数错误,请输入./ledctl run <轮数>\n");
            return -1;
        }
        static const char*const Bulb[2]={"[ ]","[■]"};
        char *endptr;//接收转换停止位置
        int a=strtol(argv[2],&endptr,10);
        if(*endptr!='\0'||endptr==argv[2]||a<=0)
        {
            printf("请输入大于0的整数\n");
            return -1;
        }
        for(int i=0;i<a;i++)
        {
            for(int j=0;j<8;j++)
            {
                char buf[9]={0};
                int reg=1<<j;
                for(int i=0;i<8;i++)
                {
                    buf[i]=(char)('0'+((reg>>(7-i))&1));
                }
                int fc=open("gpio_reg.dat",O_RDWR|O_TRUNC|O_CREAT,0644);
                if(fc<0)
                {
                    printf("打开gpio_reg.dat失败,错误原因：%s,错误位置：%d行\n",strerror(errno),__LINE__);
                    return -1;
                }
                write(fc,buf,8);
                close(fc);
                for(int k=7;k>=0;k--)
                {
                    printf("%s",Bulb[(reg>>(k))&1]);
                }
                printf("\n");
                usleep(200000);
            }
        }
    }
    else if(strcmp(argv[1],"blink")==0)
    {
        if(argc!=4)
        {
            printf("参数错误,请输入./ledctl blink <编号> <次数>\n");
            return -1;
        }
        static const char*const Bulb[2]={"[ ]","[■]"};
        int fc=open("gpio_reg.dat",O_RDWR);
        if(fc<0)
        {
            printf("打开gpio_reg.dat失败,错误原因：%s,错误位置：%d行\n",strerror(errno),__LINE__);
            return -1;
        }
        char buf[9]={0};
        read(fc,buf,8);
        char *endptr;//接收转换停止位置
        int ID=strtol(argv[2],&endptr,10);
        if(*endptr!='\0'||endptr==argv[2]||ID<0||ID>7)
        {
            printf("编号越界,0~7中其中一个编号\n");
            return -1;
        }
        int count=strtol(argv[3],&endptr,10);
        if(*endptr!='\0'||endptr==argv[3]||count<=0)
        {
            printf("请输入大于0的整数\n");
            return -1;
        }
        for(int j=0;j<count*2;j++)
        {
            int reg=0;
            buf[7-ID] ^=1;
            for(int i=0;i<8;i++)
            {
               reg=(reg<<1)|(buf[i]-'0');   
            }
            for(int k=7;k>=0;k--)
            {
                printf("%s",Bulb[(reg>>k)&1]);
            }
            lseek(fc,0,SEEK_SET);
            write(fc,buf,8);
            printf("\n");
            usleep(300000);
        }
        close(fc);
    }
    else if(strcmp(argv[1],"raw")==0)
    {
        if(argc!=3)
        {
            printf("参数错误,请输入./ledctl raw <八位二进制>\n");
            return -1;
        }
        int len=strlen(argv[2]);
        if(len!=8)
        {
            printf("请输入八位");
            return -1;
        }
        for(int i=0;i<8;i++)
        {
            if(argv[2][i]!='0'&&argv[2][i]!='1')
            {
                printf("八位数字只能是0/1");
                return -1;
            }
        }
        int fc=open("gpio_reg.dat",O_RDWR|O_CREAT|O_TRUNC,0644);
        if(fc<0)
        {
            printf("打开gpio_reg.dat失败,错误原因：%s,错误位置：%d行\n",strerror(errno),__LINE__);
            return -1;
        }
        write(fc,argv[2],8);
        close(fc);
    }
    else 
    {
        printf("请输入正确指令\n");
        return -1;
    }
    return 0;
}
