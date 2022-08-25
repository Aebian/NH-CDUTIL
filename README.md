# CDUTIL
## A disk drive utility for Windows

![CDUTIL LOGO](./media/nh-cdutil.png)

### Origin of the tool
The tool came to live, after neither standard Windows nor other programs like nirCMD had a good way of showing a status of one disk drive. <br>
Because of that, it wasn't easily possible to use the avalable features (PowerShell, nirCMD) to create a macro for my keyboard to open and close the disk drive. 

### Features

The tool features a comprehensive set of tools. 

| **Function** | **Valid Values**          | **Return**                                                                                  |   |
|--------------|---------------------------|---------------------------------------------------------------------------------------------|---|
| open         | <Drive Letter> or nothing | If <Drive Letter> specified opens that specific drive.<br>  Otherwise opens all drives found.   |   
| close        | <Drive Letter> or nothing | If <Drive Letter> specified closes that specific drive.<br>  Otherwise closes all drives found. |   
| dynamic      | <Drive Letter>            | Opens or Closes the specified drive based on the current status.                            |   |
| status       | <Drive Letter>            | Opens or Closes the specified drive based on the current status.                            |   |
| status       | <Drive Letter>            | Displays the status number of specified drive.<br>0 = Drive closed<br>1 = Drive open<br>2 = Drive closed, media loaded | 

### Example Call

``NH-CDUTIL.exe dynamic G`` or ``NH-CDUTIL.exe dynamic G:`` <br>
As pictured above, the drive can be specified either by single character or with column.


### Thanks
- [DatTestBench](https://github.com/DatTestBench) - Thanks to my pal for helping me out on C++ questions. 




#### Support My Work
- [https://aebian.org/support](https://aebian.org/support)