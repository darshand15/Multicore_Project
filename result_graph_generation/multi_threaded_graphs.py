import matplotlib.pyplot as plt
import numpy as np

# Data
categories = ['No_malloc_free', 'CPU_bound', 'Mem_bound']
malloc_values = [0.00971588, 0.497094, 0.066721]
hoard_values = [0.00994846, 0.495048, 0.0106526]
my_mem_alloc_values = [0.0095112, 0.495512, 0.215023]

# X-axis positions for the bars
x = np.arange(len(categories))

# Bar width
width = 0.2

# Create the plot
fig, ax = plt.subplots()

# Plot bars for each category
rects1 = ax.bar(x - width, malloc_values, width, label='Malloc')
rects2 = ax.bar(x, hoard_values, width, label='Hoard')
rects3 = ax.bar(x + width, my_mem_alloc_values, width, label='My_mem_alloc')

# Add text labels on top of each bar, rounded to 4 decimal places
def add_value_labels(rects):
    for rect in rects:
        height = rect.get_height()
        
        # Round the value to 4 decimal places for display
        rounded_height = round(height, 6)
        
        # Place the label directly on top of the bar
        ax.text(rect.get_x() + rect.get_width() / 2, height, 
                f'{rounded_height:.6f}', ha='center', va='bottom', fontsize=4.5, color='black')

# Add value labels to each set of bars
add_value_labels(rects1)
add_value_labels(rects2)
add_value_labels(rects3)

# Set labels and title
ax.set_xlabel('Type of Benchmark Program', fontweight='bold')
ax.set_ylabel('Execution Time (in seconds)', fontweight='bold')
ax.set_title('Speed Comparison of Memory Allocators (Multi Threaded)', fontweight='bold')
ax.set_xticks(x)
ax.set_xticklabels(categories)

# Set y-axis to logarithmic scale
# ax.set_yscale('log')

# The y-axis will automatically use scientific notation (powers of 10)

# Add legend
ax.legend()

# Save the plot as a PNG image
plt.savefig('multi_threaded_speed.png', dpi=300, bbox_inches='tight')

# Show the plot
# plt.show()
