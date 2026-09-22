import pandas as pd
import matplotlib.pyplot as plt

# Load empirical data
data = pd.read_csv('benchmark_results.csv')

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))

# Graph 1: Time Scalability Proof
ax1.plot(data['Operations'], data['Avg_Latency_Microseconds'], marker='o', color='blue', linewidth=2)
ax1.set_title('O(1) Scalability: Latency vs. Workload')
ax1.set_xlabel('Number of Requests')
ax1.set_ylabel('Avg Latency (Microseconds)')
ax1.set_xscale('log') # Log scale handles 10M spread beautifully
ax1.set_ylim(0, max(data['Avg_Latency_Microseconds']) * 2) 
ax1.grid(True)

# Graph 2: Memory Tax
ax2.plot(data['Operations'], data['Memory_Overhead_Bytes'] / (1024 * 1024), marker='s', color='red')
ax2.set_title('System Memory Overhead')
ax2.set_xlabel('Number of Requests')
ax2.set_ylabel('Memory Overhead (MB)')
ax2.set_xscale('log')
ax2.grid(True)

plt.tight_layout()
plt.savefig('benchmark_graphs.png')
print("Saved benchmarking output to 'benchmark_graphs.png'")
plt.show()