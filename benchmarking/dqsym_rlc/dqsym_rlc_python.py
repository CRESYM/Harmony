import argparse
import os
import time
import numpy as np


def peak_memory_mb():
    if os.name == "nt":
        import ctypes
        from ctypes import wintypes
        class PMC(ctypes.Structure):
            _fields_ = [("cb", wintypes.DWORD), ("PageFaultCount", wintypes.DWORD),
                        ("PeakWorkingSetSize", ctypes.c_size_t), ("WorkingSetSize", ctypes.c_size_t),
                        ("QuotaPeakPagedPoolUsage", ctypes.c_size_t), ("QuotaPagedPoolUsage", ctypes.c_size_t),
                        ("QuotaPeakNonPagedPoolUsage", ctypes.c_size_t), ("QuotaNonPagedPoolUsage", ctypes.c_size_t),
                        ("PagefileUsage", ctypes.c_size_t), ("PeakPagefileUsage", ctypes.c_size_t)]
        counters = PMC(); counters.cb = ctypes.sizeof(counters)
        kernel32, psapi = ctypes.windll.kernel32, ctypes.windll.psapi
        kernel32.GetCurrentProcess.restype = wintypes.HANDLE
        psapi.GetProcessMemoryInfo.argtypes = [wintypes.HANDLE, ctypes.POINTER(PMC), wintypes.DWORD]
        psapi.GetProcessMemoryInfo.restype = wintypes.BOOL
        ok = psapi.GetProcessMemoryInfo(kernel32.GetCurrentProcess(), ctypes.byref(counters), counters.cb)
        if not ok:
            raise ctypes.WinError()
        return counters.PeakWorkingSetSize / 2**20
    import resource
    value = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss
    return value / (2**20 if os.uname().sysname == "Darwin" else 1024)


def transform_blocks(M, S, Sinv):
    out = np.zeros_like(M, dtype=np.complex128)
    for i in range(M.shape[0] // 3):
        for j in range(M.shape[1] // 3):
            r, c = 3*i, 3*j
            out[r:r+3, c:c+3] = S @ M[r:r+3, c:c+3] @ Sinv
    return out


def dqn2abc(X, theta, Sinv):
    abc = np.real(Sinv @ X[:, 0])
    for h in range(1, X.shape[1]):
        th = h * theta
        mp, mn, mz = np.abs(X[:, h]); ap, an, az = np.angle(X[:, h])
        abc += mp*np.sin(th+ap+np.array([0, -2*np.pi/3, 2*np.pi/3]))
        abc += mn*np.sin(th+an+np.array([0, 2*np.pi/3, -2*np.pi/3]))
        abc += mz*np.sin(th+az)
    return abc


def matrices():
    A = np.array([
        [.999055,0,0,-.00900385,0,0],[0,.999055,0,0,-.00900385,0],
        [0,0,.999055,0,0,-.00900385],[.00999427,0,0,.999755,0,0],
        [0,.00999427,0,0,.999755,0],[0,0,.00999427,0,0,.999755]], complex)
    B = np.array([[450.238,0,0],[0,450.238,0],[0,0,450.238],
                  [2.25096,0,0],[0,2.25096,0],[0,0,2.25096]], complex)
    C = np.array([
        [9.99427e-8,0,0,1.99976e-5,0,0],[0,9.99427e-8,0,0,1.99976e-5,0],
        [0,0,9.99427e-8,0,0,1.99976e-5],[-1.99905e-5,0,0,9.00385e-8,0,0],
        [0,-1.99905e-5,0,0,9.00385e-8,0],[0,0,-1.99905e-5,0,0,9.00385e-8]], complex)
    D = np.array([[2.25096e-5,0,0],[0,2.25096e-5,0],[0,0,2.25096e-5],
                  [-.00450238,0,0],[0,-.00450238,0],[0,0,-.00450238]], complex)
    u = np.array([
        [0,-245.5-880.2j,-102.1+319.4j,373.9+733.8j,255.1-885.1j,161.1+983.9j],
        [0,-216.5+514.1j,-400.2+211.4j,-82.4-141.3j,-536.9+212.4j,-515.6-399.9j],
        [0,-119.4-115.7j,-335.7-128.8j,-889.6-92.9j,3.2-257.6j,242.7+301.7j]], complex)
    return A, B, C, D, u


def simulate(t_end=0.5, dt=2e-5, f0=50.0):
    A, B, C, D, u = matrices()
    a = -.5 + .866j; S = np.array([[1,a,np.conj(a)],[1,np.conj(a),a],[1,1,1]], complex)/3
    Sinv = np.linalg.inv(S)
    A0, B0 = transform_blocks(A,S,Sinv), transform_blocks(B,S,Sinv)
    C0, D0 = transform_blocks(C,S,Sinv), transform_blocks(D,S,Sinv)
    rot = np.exp(-1j*2*np.pi*f0*dt*np.arange(u.shape[1]))
    n = int(t_end/dt)+1
    time_hist = np.arange(n)*dt
    brk = np.zeros((n,3), np.int32)
    abc = np.zeros((2,n,3))
    x_old = np.zeros((6,6), complex)
    start = time.perf_counter()
    for k,t in enumerate(time_hist):
        brk[k,:] = 1 if 2e-4 <= t < 6e-4 else 0  # swType=0: recorded only
        x_old = (A0 @ x_old + B0 @ u) * rot
        y = C0 @ x_old + D0 @ u
        theta = 2*np.pi*f0*t
        abc[0,k] = dqn2abc(y[:3],theta,Sinv)
        abc[1,k] = dqn2abc(y[3:],theta,Sinv)
    elapsed = time.perf_counter()-start
    return elapsed, peak_memory_mb(), x_old, abc, brk


if __name__ == "__main__":
    p=argparse.ArgumentParser(); p.add_argument("--t-end",type=float,default=.5); p.add_argument("--repeats",type=int,default=5)
    args=p.parse_args(); runs=[]; result=None
    for _ in range(args.repeats): result=simulate(args.t_end); runs.append(result[0])
    print(f"Python/NumPy DQsym RLC: steps={int(args.t_end/2e-5)+1}, median_time_s={np.median(runs):.9f}, peak_memory_mb={result[1]:.3f}")
    print("final_state_checksum=%.16e" % np.sum(np.abs(result[2])))
