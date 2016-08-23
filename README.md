# coaping
=======

A ping for CoAP, the constrained Application Protocol **written in C**.

### First things first
First of all, I want to say that there is a code with the same name written for ruby by [**cabo**](https://github.com/cabo) that does more or less the same as my code does. But he did it before me. You can find it [**here**](https://github.com/cabo/coaping).

### Examples

- Send three CoAP empty `CON` datagrams to vs0.inf.ethz.ch, port 12345:
```
coaping -n 3 -p 12345 vs0.inf.ethz.ch
```
- Send infinite CoAP empty `CON` datagrams to vs0.inf.ethz.ch, port 57278:
```
coaping vs0.inf.ethz.ch
coaping -n 0 vs0.inf.ethz.ch
```

The expected output for the last example should look like this:

```

PING idvm-infk-mattern04.inf.ethz.ch (129.132.15.80)	port: 5683 
4 bytes from idvm-infk-mattern04.inf.ethz.ch (129.132.15.80): type=RST time=29.39 ms 
4 bytes from idvm-infk-mattern04.inf.ethz.ch (129.132.15.80): type=RST time=30.51 ms 
4 bytes from idvm-infk-mattern04.inf.ethz.ch (129.132.15.80): type=RST time=31.84 ms 
4 bytes from idvm-infk-mattern04.inf.ethz.ch (129.132.15.80): type=RST time=30.55 ms 
4 bytes from idvm-infk-mattern04.inf.ethz.ch (129.132.15.80): type=RST time=30.25 ms 
4 bytes from idvm-infk-mattern04.inf.ethz.ch (129.132.15.80): type=RST time=29.27 ms 
4 bytes from idvm-infk-mattern04.inf.ethz.ch (129.132.15.80): type=RST time=28.96 ms 
^C
--- 129.132.15.80 ping statistics ---
7 packets transmitted, 7 received, 0 errors, 0% packet loss

```
