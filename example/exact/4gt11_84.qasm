OPENQASM 2.0;
include "qelib1.inc";
qreg q[16];
creg c[16];
h q[0];
t q[2];
t q[1];
t q[0];
cx q[1],q[2];
cx q[0],q[1];
cx q[2],q[0];
tdg q[1];
cx q[2],q[1];
tdg q[2];
tdg q[1];
t q[0];
cx q[0],q[1];
cx q[2],q[0];
cx q[1],q[2];
h q[0];
//cx q[4],q[0];
//cx q[0],q[4];
cx q[3],q[0];
cx q[0],q[3];
