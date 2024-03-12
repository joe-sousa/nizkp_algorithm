# Towards a Comparative Study of Authentication Mechanisms for Low-Resource Internet of Things Devices

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



