Intel(R) Quartus(R) Prime Design Suite Standard Edition Version 22.1 README.TXT
===============================================================================

Although we have made every effort to ensure that this version of the Intel FPGA
software operates correctly, you can encounter problems that we have not 
discovered. If you have a question or problem that is not answered by the 
information provided in this readme.txt file, go to 
https://www.intel.com/content/www/us/en/support/programmable/support-resources/overview.html
for additional support resources, or submit a service request at 
https://premiersupport.intel.com.

This readme.txt file contains the following information:
* Intel Quartus Prime Design Suite Contents
   Lists the set of Intel design tools.
* Technical Documentation
   Directs you where to find documentation for Intel design tools.
* System Requirements
   Lists the system requirements.
* Useful Links
   Lists links to other Intel Quartus Prime resources including videos and 
   reference designs.
* Notices
   Trademark and other legal notices that apply to this document.


Intel Quartus Prime Design Suite Standard Edition Contents
==========================================================
*  Intel Quartus Prime Design Standard Edition Software
   (includes the Intel FPGA IP Library and Intel Nios(R) II Embedded Design Suite)
*  Questa(*)- Intel FPGA Edition simulation tool
*  Questa - Intel FPGA Starter Edition simulation tool
*  Intel FPGA SoC Embedded Development Suite (Pro and Standard Editions)

You can download the Intel Quartus Prime software, as well as tools such 
as the Advanced Link Analyzer or Stand-Alone Programmer, from the
Intel FPGA and Programmable Devices Download Center on the Intel website:
https://www.intel.com/content/www/us/en/collections/products/fpga/software/downloads.html


Technical Documentation
=======================

Release Notes
-------------
*  Intel Quartus Prime Standard Edition Software and Device Support Release Notes
   Provides information about new features and enhancements, known issues,
   workarounds, device support information, memory requirements for design
   processing, and timing and power model status. This document is available on 
   the Intel FPGA website:
   https://www.intel.com/content/www/us/en/docs/programmable/683593

*  Intel FPGA IP Release Notes
   Provides information about new features and enhancements, errata, and
   workarounds. For a complete list of all Intel FPGA IP release notes, see the 
   Intel FPGA and Programmable Devices Release Notes page on the Intel FPGA 
   website:
   https://www.intel.com/content/www/us/en/support/programmable/support-resources/fpga-documentation-index.html?s=Newest&f:@stm_10386_en=%5BIntellectual%20Property%5D&f:@emtcontenttype_en=%5BRelease%20Notes%5D
   
*  Intel Nios II Release Notes
   Provides information about new features, enhancements, bug fixes, and
   known issues for the Nios(R) II Embedded Design Suite (EDS), Nios(R) 
   Processor IP, and Embedded Intellectual Property (IP). This document is 
   available on the Intel FPGA website:
   https://www.intel.com/content/www/us/en/docs/programmable/683482/
   
*  Intel SoC Embedded Development Suite Release Notes
   Provides information about new features, enhancements, bug fixes and known 
   issues for both Pro and Standard Editions. This document is available on the
   Intel FPGA website:
   https://www.intel.com/content/dam/altera-www/global/en_US/pdfs/literature/rn/rn_soc_eds.pdf

Errata
------
   You can find information, errata, and workarounds by searching the Intel FPGA
   Knowledge Base:
   https://www.intel.com/content/www/us/en/support/programmable/kdb-filter.html

Other Technical Documentation
-----------------------------
*  Intel Quartus Prime Software Download and Installation Quick Start Guide
   Provides directions on how to download and install both Windows and Linux
   versions of the Quartus Prime Design Suite software. This document is 
   available on the Intel FPGA website:
   https://www.intel.com/content/www/us/en/docs/programmable/683533/

*  Intel FPGA Software Installation and Licensing Manual
   Provides information about installing and licensing your Intel FPGA design
   software. This document is available on the Intel FPGA website:
   https://www.intel.com/content/www/us/en/docs/programmable/683472/

*  Introduction to the Intel Quartus Prime Software
   Provides an introduction to using the Intel Quartus Prime software. This 
   document is available on the Intel FPGA website:
   https://www.intel.com/content/dam/altera-www/global/en_US/pdfs/literature/br/br-quartus-prime-software.pdf

*  Intel Quartus Prime User Guides
   Provides detailed information about all aspects of
   the Intel Quartus Prime software. These documents are available on the Intel
   FPGA website:
   https://www.intel.com/content/www/us/en/support/programmable/support-resources/design-software/user-guides.html

*  AN 320: Intel FPGA IP Evaluation Mode
   https://www.intel.com/content/www/us/en/docs/programmable/683502/


System Requirements
===================

Operating system support
------------------------
Intel Quartus Prime Design Suite components support one or more of the following
operating systems:

   *  CentOS(*) Linux 8.2
   *  Red Hat(*) Enterprise Linux(*) 8.2
   *  SUSE(*) Linux Enterprise Server 12 SP5
   *  SUSE Linux Enterprise Server 15 SP3
   *  Ubuntu(*) 16.04 LTS
   *  Ubuntu 18.04 LTS
   *  Ubuntu 20.04 LTS
   *  Windows(*) 10 (version 1607 or later)
   *  Windows Server(*) 2012
   *  Windows Server 2016
   *  Windows Server 2019

Review the notes that follow to check operating system support for the 
components that you use.

Notes:
  * Only 64-bit operating systems are supported.
  * Intel Quartus Prime Standard and Lite editions are supported on all listed 
    operating systems.
  * Questa - Intel FPGA Edition and Questa - Intel FPGA Starter Edition are 
    supported only on the following operating systems:
         *  Red Hat Enterprise Linux 8
         *  SUSE Linux Enterprise Server 12
         *  SUSE Linux Enterprise Server 15
         *  Windows 10
	  Red Hat Enterprise Linux operating systems require 32-bit library files.
   * Nios II EDS Standard Edition is supported only on the following operating 
     systems:
         *  SUSE Linux Enterprise Server 12
         *  Windows 10
  * Intel Quartus Prime Stand-Alone Programmer Standard Editions is 
    supported on all listed operating systems.
  * Intel Advanced Link Analyzer is supported only on Windows operating systems.

Operating System Support information including an updated list of limitations 
for all editions of the Intel Quartus Prime software can be found on the 
Intel FPGA website:
https://www.intel.com/content/www/us/en/programmable/support/support-resources/download/os-support.html

FLEXlm
------
   If you are using a floating license server for Intel FPGA software, ensure 
   that you use latest license daemon software available from the following URL:
   https://www.intel.com/content/www/us/en/software-kit/709649/
   
   For more information about the FLEXlm license server software, refer to the 
   Intel FPGA Software Installation and Licensing manual:
   https://www.intel.com/content/www/us/en/docs/programmable/683472/

KDE or GNOME Display Manager
----------------------------
   *  Version included with your Linux operating system installation.


Useful Links
============
*  You can create a My Intel account where you can file a service request,
   register for a class, download software, and more on the Intel website.
   https://www.intel.com/content/www/us/en/secure/my-intel/dashboard.html

*  You can view video demonstrations for Intel Quartus Prime and Nios II 
   software on the Intel FPGA Quick Videos website:
   https://www.intel.com/content/www/us/en/support/programmable/support-resources/fpga-training/fpga-quick-video-index.html

*  You can download reference designs that show efficient solutions for common 
   system design problems from Intel FPGA and our partners on the
   Intel FPGA Design Examples website:
   https://www.intel.com/content/www/us/en/support/programmable/support-resources/design-examples/design-examples-overview.html
   Further design examples are available in the Intel FPGA Design Store:
   https://www.intel.com/content/www/us/en/support/programmable/support-resources/design-examples/design-store.html

*  You can compare the features of the Pro, Standard and Lite Editions of the 
   Intel(R) Quartus(R) Prime Software on the Intel FPGA website:
   https://www.intel.com/content/www/us/en/software/programmable/quartus-prime/download.html

*  You can ask questions or share your knowledge about Intel Quartus Prime 
   Software in the forums available at the Intel Communities site:
   https://community.intel.com/t5/Intel-Quartus-Prime-Software/bd-p/quartus-prime-software  


Notices
=======
This product includes software derived from the RSA Data Security, Inc. 
MD5 Message-Digest Algorithm.

Intel, Nios, and Quartus words and logos are trademarks of Intel Corporation 
or its subsidiaries in the U.S. and/or other countries.

Other names and brands may be claimed as the property of others.