# systimer infoleak

This poc will leak the win32kfull!CaretBlinkProc (nt4 - win10)

很好用的一个信息泄露,去年pwn2own的时候没舍得用。
今天整理漏洞的时候发现不能用了,往前回溯了下,应该是11月份的补丁.

感觉应该是这个.
````
Windows Kernel Information Disclosure Vulnerability	CVE-2017-11851	
Marcin Wiazowski working with Trend Micro's Zero Day Initiative
````

补的地方也很简单
补之前:
````c
__int64 __usercall xxxRealInternalGetMessage()
{
    ...
              v82 = 0x118i64;
              if ( !(*(_DWORD *)(v73 + 48) & 2) )
                v82 = 0x113i64;
              LODWORD(v118) = 0;
              LODWORD(v117) = 0;
              LODWORD(v116) = 0;
              StoreQMessage(
                v76,
                *(_QWORD *)(v73 + 0x58),
                v82,
                *(_QWORD *)(v73 + 0x60),
                *(_QWORD *)(v73 + 0x20), //win32kfull!CaretBlinkProc
                v116,
                0i64,
                v117,
                0i64,
                v118,
                0i64,
                v81,
                0i64,
                0i64);
    ...
}

````

补之后

````c
__int64 __usercall xxxRealInternalGetMessage()
{
           v77 = 0x118i64;
            if ( !(*(_DWORD *)(v64 + 0x30) & 2) )
              v77 = *(_QWORD *)(v64 + 0x20);
            v78 = 0x118;
            if ( !(*(_DWORD *)(v64 + 0x30) & 2) )
              v78 = 0x113;
            StoreQMessage(
              v69,
              *(_QWORD **)(v64 + 0x58),
              v78,
              *(_QWORD *)(v64 + 0x60),
              v77,//变成118了,地址没了.
              0,
              0i64,
              0,
              0i64,
              0,
              0i64,
              v76,
              0i64,
              0i64);
}
````
