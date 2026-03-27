import re
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd

ROOT = Path.home() / "parallel_lab" / "lab1"
RESULT = ROOT / "result"
FIG = ROOT / "figures"
FIG.mkdir(exist_ok=True)

# ---------- 读 CSV ----------
matvec = pd.read_csv(RESULT / "matvec_bench.csv")
sumbench = pd.read_csv(RESULT / "sum_bench.csv")

# ---------- 工具函数 ----------
def extract_first_int(pattern, text):
    m = re.search(pattern, text.replace(",", ""))
    return int(m.group(1)) if m else None

def extract_first_float(pattern, text):
    m = re.search(pattern, text.replace(",", ""))
    return float(m.group(1)) if m else None

def read_text(path):
    return Path(path).read_text(encoding="utf-8", errors="ignore")

# ---------- 图 A：matvec 时间曲线 ----------
plt.figure(figsize=(6.5, 4.2))
plt.plot(matvec["n"], matvec["naive_ms"], marker="o", label="naive")
plt.plot(matvec["n"], matvec["opt_ms"], marker="s", label="opt")
plt.xscale("log", base=2)
plt.yscale("log")
plt.xlabel("n")
plt.ylabel("Average time (ms)")
plt.title("Matvec: naive vs optimized")
plt.legend()
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig(FIG / "A_matvec_time.png", dpi=200)
plt.close()

# ---------- 图 B：matvec L1 miss ratio ----------
naive_l1 = read_text(RESULT / "matvec_l1_naive.txt")
opt_l1 = read_text(RESULT / "matvec_l1_opt.txt")

naive_loads = extract_first_int(r"([0-9,]+)\s+cpu_core/L1-dcache-loads/", naive_l1)
naive_misses = extract_first_int(r"([0-9,]+)\s+cpu_core/L1-dcache-load-misses/", naive_l1)
opt_loads = extract_first_int(r"([0-9,]+)\s+cpu_core/L1-dcache-loads/", opt_l1)
opt_misses = extract_first_int(r"([0-9,]+)\s+cpu_core/L1-dcache-load-misses/", opt_l1)

naive_ratio = 100.0 * naive_misses / naive_loads
opt_ratio = 100.0 * opt_misses / opt_loads

plt.figure(figsize=(5.6, 4.2))
plt.bar(["naive", "opt"], [naive_ratio, opt_ratio])
plt.ylabel("L1 load miss ratio (%)")
plt.title("Matvec at n=1024")
for i, v in enumerate([naive_ratio, opt_ratio]):
    plt.text(i, v + 0.8, f"{v:.2f}%", ha="center")
plt.tight_layout()
plt.savefig(FIG / "B_matvec_l1_miss_ratio.png", dpi=200)
plt.close()

# ---------- 图 C：sum 时间曲线 ----------
plt.figure(figsize=(6.5, 4.2))
plt.plot(sumbench["n"], sumbench["chain_ms"], marker="o", label="chain")
plt.plot(sumbench["n"], sumbench["twoway_ms"], marker="s", label="twoway")
plt.xscale("log", base=2)
plt.yscale("log")
plt.xlabel("n")
plt.ylabel("Average time (ms)")
plt.title("Reduction: chain vs twoway")
plt.legend()
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig(FIG / "C_sum_time.png", dpi=200)
plt.close()

# ---------- 图 D：sum IPC 柱状图 ----------
def parse_instr_cycles(path):
    txt = read_text(path)
    instr = extract_first_int(r"([0-9,]+)\s+cpu_core/instructions/", txt)
    cyc = extract_first_int(r"([0-9,]+)\s+cpu_core/cycles/", txt)
    return instr, cyc, instr / cyc

chain_4096 = parse_instr_cycles(RESULT / "sum_perf_chain_4096.txt")
twoway_4096 = parse_instr_cycles(RESULT / "sum_perf_twoway_4096.txt")
chain_16384 = parse_instr_cycles(RESULT / "sum_perf_chain_16384.txt")
twoway_16384 = parse_instr_cycles(RESULT / "sum_perf_twoway_16384.txt")

labels = ["chain-4096", "twoway-4096", "chain-16384", "twoway-16384"]
ipcs = [chain_4096[2], twoway_4096[2], chain_16384[2], twoway_16384[2]]

plt.figure(figsize=(7.2, 4.2))
plt.bar(labels, ipcs)
plt.ylabel("IPC")
plt.title("Reduction IPC comparison")
for i, v in enumerate(ipcs):
    plt.text(i, v + 0.03, f"{v:.2f}", ha="center")
plt.tight_layout()
plt.savefig(FIG / "D_sum_ipc.png", dpi=200)
plt.close()

print("Figures saved to:", FIG)
print((FIG / "A_matvec_time.png"))
print((FIG / "B_matvec_l1_miss_ratio.png"))
print((FIG / "C_sum_time.png"))
print((FIG / "D_sum_ipc.png"))
