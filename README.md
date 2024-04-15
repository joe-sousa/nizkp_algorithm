# Towards a Comparative Study of Authentication Mechanisms for Low-Resource Internet of Things Devices

**Preparing the Development Environment and Dependencies**

## Cloning the Repository 📥:

1. Clone the repository:

git clone https://github.com/joe-sousa/nizkp_algorithm.git

## Downloading Arduino IDE 🛠️
For Linux:
Open a web browser and go to the Arduino Software page.
Click on the "Linux 64 bits" or "Linux 32 bits" download link, depending on your system architecture.
Once the download is complete, navigate to the directory where the downloaded file is located.
Extract the downloaded archive using the following command in the terminal:
php

tar -xvf arduino-<version>-linux64.tar.xz

Replace <version> with the version number of the Arduino IDE you down

Navigate to the extracted folder and run the install.sh script:
bash
Copy code
cd arduino-<version>
sudo ./install.sh

Follow the on-screen instructions to complete the installation process.

For Windows:
Open a web browser and go to the Arduino Software page.
Click on the "Windows Installer" download link.
Once the download is complete, locate the downloaded .exe file and double-click to run it.
Follow the on-screen instructions in the installation wizard.
Choose the installation directory and any additional options you prefer.
Complete the installation process by clicking "Install" or "Finish" as prompted.
After following these instructions, you should have the Arduino IDE successfully installed on your Linux or Windows system.

## Arduino IDE Configuration ⚙️

**Selecting the Correct Board**
Navigate to Tools -> Board -> Arduino Nano to choose the appropriate board that will be utilized.

**Selecting the Correct Processor**
Open the Arduino IDE.
Go to Tools -> Processor.
From the dropdown menu, select "Atmega328P(OldBootLoader)".
Ensure that the correct processor is now selected for your project.

**Configuring Serial Monitor Baud Rate**
Before running the program containing the ZKP algorithm, ensure to select the correct baud rate in the serial monitor. For optimal performance, choose the baud rate of 115200. This ensures smooth communication and accurate data transmission between the Arduino and the connected device.

After running the program containing the ZKP algorithm, you should receive a message similar to the following, indicating a successful operation:
"Sketch uses 25832 bytes (84%) of program storage space. Maximum is 30720 bytes.
Global variables use 766 bytes (37%) of dynamic memory, leaving 1282 bytes for local variables. Maximum is 2048 bytes."

## Installing Libraries 📚:
Arduino libraries are collections of code that enable you to easily connect to sensors, displays, modules, etc. Follow these steps to install additional libraries in the Arduino IDE:

Open the Arduino IDE.
Navigate to Sketch -> Include Library -> Manage Libraries....
In the Library Manager, find the library you want to install by scrolling or searching.
Click on the library and select the version you want to install (if applicable).
Click Install and wait for the installation to complete.
Once installed, the library will appear in the Sketch -> Include Library menu.
Alternatively, you can import a library distributed as a ZIP file by navigating to Sketch -> Include Library -> Add .ZIP Library....
For manual installation, download the library as a ZIP file, extract it, and place the folder containing the library files in the "libraries" folder inside your Arduino sketchbook directory.


## Motivation 💡:
Authenticity is a critical aspect of information security, especially in the realm of Internet of Things (IoT) devices within Industry 4.0. 
However, deploying authentication mechanisms on specific IoT devices presents significant challenges, particularly concerning energy, memory,
and computational capacity constraints. In this context, our ongoing research project aims to compare various authentication mechanisms tailored
for low-computational-power IoT devices. The primary objective is to identify the most efficient solution. This paper seeks to present our methodological
scope and preliminary results derived from a brief computational experiment. Our initial findings demonstrate the practical viability of executing the 
Non-Interactive Zero-Knowledge Proof (NIZKP) algorithm on Arduino Nano. This work contributes to the literature and practice of IoT device authentication,
offering insights that can aid in better decision-making processes in industrial scenarios.

## 📍 Proposal
In light of this motivation, this ongoing research project aims to conduct a robust comparison among different authentication mechanisms in the context of
low-resource IoT devices with the goal of identifying the most efficient one. Specifically, we seek to investigate whether NIZKP over elliptic curves demands 
fewer computational resources compared to conventional authentication mechanisms, that is: Hash-based Message Authentication Code (HMAC), Advanced Encryption
Standard (AES), and Rivest, Shamir and Adleman (RSA).

## Methodological Scope 📊:

This research adopts an experimental approach with a descriptive focus. We outline the methodological scope through a computational experiment, which involves
a comparative analysis of authentication mechanisms in low-resource IoT devices. Specifically, the study will compare the performance of NIZKP over elliptic curves
with that of HMAC, AES, and RSA on the Arduino Nano platform. Our primary objectives include investigating computational processing time (in milliseconds), memory 
usage (in kilobytes), and energy consumption (in millijoules). The experimental design is visually depicted in a Figure to enhance the clarity of the methodological scope.

![model architecture](https://i.ibb.co/fpgpN0G/scope-methodology-paper-version.png)

## Preliminary Results 📈:

The behavior of the algorithm during the key generation pro-
cess, highlighting the time in milliseconds and energy in millijoules consumed during the
tests. Initially, we aimed to estimate the average time for generating the public and private
key pair. This analysis revealed that the algorithm required an average of 3739.07 ms for
this task, with a standard deviation of 3.86 ms, offering significant insights into the algo-
rithm’s efficiency in this regard. The subsequent analysis centered on quantifying energy
consumption, allowing us to estimate the energy consumption for each experiment execu-
tion using the established equation. Lastly, in the concluding assessment, we scrutinized
memory usage. It was observed that the algorithm consumed 25832 bytes, representing
84% of the available Flash memory space (30720 bytes). Furthermore, the experiment
revealed that the NIZKP algorithm utilized 766 bytes of dynamic SRAM memory, ac-
counting for 37% of the total available space (2048 bytes).
Figure 2. Estimation of Time and Energy Consumption for Key Pair Generation in
NIZKP.




