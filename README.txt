Kevin Johnson #101077070
Jack Hendry   #101079591

**************************Term Project****************************************


Files Included---------------------------------------------------------------

C files:
atm1.c
atm2.c
atm3.c
atm4.c
atm5.c
dbServer.c
atmDecide.c
dbEditor.c
Interest.c
header.h
atm1DeadLock.c
atm2DeadLock.c
atm1LiveLock.c
atm2LiveLock.c
atm1Starvation.c
atm2Starvation.c

Executables:
atm1EXE.txt
atm2EXE.txt
atm3EXE.txt
atm4EXE.txt
atm5EXE.txt
dbServerEXE.txt
atmDecideEXE.txt
dbEditorEXE.txt
InterestEXE.txt
atm1DeadLockEXE.txt
atm2DeadLockEXE.txt
atm1LiveLockEXE.txt
atm2LiveLockEXE.txt
atm1StarvationEXE.txt
atm2StarvationEXE.txt



IMPORTANT TEXT FILES: 
db.txt 
logDL.txt
logLL.txt 
logS.txt

IMPORTANT Before Running -------------------------------------------------------------------------
Make sure there is at least one entry in the db.txt file this should be added
manually or the server wont work correctly. entries can be added and edited 
through the use of the editor after this. adding an initial entry wasn’t
specified in the pdf so we only added functionality for adding and editing entries
after the initial
  

----------------------------------------------------------------------------------------
DISCLAIMER: 
IF GETTING WEIRD OUTPUT OR THINGS ARE NOT WOKRING AS THEY SHOULD, THIS IS MOST LIKELY DUE TO BACKAGE IN THE MESSAGE Q’S. 
This rarely happen but can still occur.
TO ADDRESS THIS JUST CLICK X WHEN ENTER AN ACCOUNT IS PROMPTED TO CLOSE ATMS, SERVER, AND ATMDECIDE AND IT SHOULD RESOLVE

----------------------------------------------------------------------------------------


How to run-------------------------------------------------------------------------------

1. run the server
2. run the atmDecide

Order doesn’t matter, run them all or run individually:
3. run as many ATMs as you want order doesn’t matter 
4. run iterator
5. run dB Editor

How to use ATMs---------------------------------------------------------------------------
1. Enter in the correct Account number put in X to close all the system
2. The atm pin is the pin listed in the database + 1. We assumed that’s how the masking works so if
   db. says pin is 500 then enter 501 in the atm.
3. The atm will display a pin right and you can enter: W or D or C (Withdraw, Deposit, Chequing).
   if C is entered your current balance will print out 
4 if W or D is entered then you must enter the amount of funds to withdraw or deposit
  there are checks put in place so you cant withdraw a negative balance but you can deposit to a negative balance
5. You can run all 5 ATMs in any order you want


Things to know running 1 atm:
-if an account doesn’t exist in the db this will result in the atm 
 starting again by asking you for account and pin
-if an account exists but a pin doesn’t then the atm will keep asking for the pin 
 until it is correct or the account is blocked. The account is blocked after 3 attempts
 causing the atm to restart from the beginning
- if an account gets blocked the atm will restart asking for account and pin

Other things to know running multiple ATMs:
- when running multiple ATMs, they can all send account and pin but will wait until the 
  first atm that ran finish’s execution before the other
-the only way the next atm can run is if the first finishes or the first put in the wrong
 account, pin, or account is blocked resulting in the next ATMs message in the message queue 
 to be processed
- when MULTIPLE atm messages and MULTIPLE editor messages are sent, the editor messages will
  always be processed in-between atm messages even if ATMs send messages back-to-back, then an editor 
  sends messages. Output will reflect this pattern atm1, editor, atm2, editor

How to use editor-----------------------------------------------------------------------------------------------
1. follow prompts on screen to add or edit file
2. to edit type in the account number, pin doesn’t matter, then add in new funds

How to use Interest---------------------------------------------------------------------------------------------
1. just run it to add interest every 1-minute doesn’t use mutual exclusion with the other
   processes since it would take precedence and stop the rest in the middle of message 
   processing which could lead to messages being lost.

------------------------------------------------------------------------------------------------------------------
How to generate Deadlock: 
1.	Run the following executables; atm1DeadLockEXE, atm2DeadLockEXE, dbServerEXE,atmDecideEXE.
2.	atm1DeadLockEXE, atm2DeadLockEXE, will write to logDL.txt so have it open. 
3.	Enter the account and pin into terminal of atm1DeadLockEXE.
4.	Enter the account and pin into terminal of atm2DeadLockEXE.
5.	Look at the newly added text, in logDL.txt. 
6.	Be patient it may take some time to write.  

------------------------------------------------------------------------------------------------------------------

How to generate Livelock: 
1.	Run the following executables; atm1LiveLockEXE., atm2LiveLockEXE, dbServerEXE, atmDecideEXE.
2.	atm1LiveLockEXE, atm2DeadLiveEXE, will write to logLL.txt so have it open. 
3.	Enter the account and pin into terminal of atm1LiveLockEXE.
4.	Enter the account and pin into terminal of atm2LiveLockEXE.
5.	Look at the newly added text, in logLL.txt. 
6.	Be patient it may take some time to write put it will, also it will go on for ever. 


--------------------------------------------------------------------------------------------------------------------


How generate Starvation: (NOTE: RUN atm1StarvationEXE FIRST, LET atm2StarvationEXE completly run or prints won't show up) 
1.	Run the following executables; atm1StarvationEXE, atm2StarvationEXE, dbServerEXE,atmDecideEXE.
2.	atm1StarvationEXE, atm2StarvationEXE will write to logS.txt so have it open. 
3.	Enter the account and pin into terminal of atm1StarvationEXE.
4.	Enter the account and pin into terminal of atm2StarvationEXE.
5.	Look at the newly added text, in logS.txt. 
6.	Be patient it may take some time to write.  

--------------------------------------------------------------------------------------------------------------------


 





