# AD9986 API 

## 1. INTRODUCTION

### 1.1 PURPOSE
This document serves as a programmer's reference for using and utilizing various aspects of the Application Program Interface (API) library targeting the AD9986 family of ADI Direct RF Transmitter and Receiver Devices. It describes the general structure of the AD9986 API library, provides a detail list of the API functions and its associated data structures, macros, and definitions. 

### 1.2 SCOPE
This document targets API libraries the devices listed in following table, a source code package is available for each devices.

|Target Device Name | Device Description                                                   | Device Release Status|
|-------------------|----------------------------------------------------------------------|----------------------|
|AD9081             | 4x 16-bit 12GSPS RF DAC cores and 4x 12- bit 4GSPS rate RF ADC cores | Unreleased           |
|AD9082             | 4x 16-bit 12GSPS RF DAC cores and 2x 12- bit 6GSPS rate RF ADC cores | Unreleased           |
|AD9986             | 4x 16-bit 12GSPS RF DAC cores and 2x 12- bit 6GSPS rate RF ADC cores | Unreleased           |
|AD9988             | 4x 16-bit 12GSPS RF DAC cores and 4x 12- bit 4GSPS rate RF ADC cores | Unreleased           |

### 1.3 DISCLAIMER
This is a preliminary pre-release version of API and documentation. All information is subject to change.
The software and any related information and/or advice is provided on and "AS IS" basis, without representations, guarantees or warranties of any kind, express or implied, oral or written, including without limitation warranties of merchantability fitness for a particular purpose, title and non-infringement. Please refer to the Software License Agreement applied to the source code for full details.

## 2. ARCHITECTURE
![Linux Server Based Arch](https://confluence.analog.com/download/attachments/21520831/Evaluation%20System%E2%80%99s%20Software%20Architecture7.PNG?version=1&modificationDate=1516009214932&api=v2)