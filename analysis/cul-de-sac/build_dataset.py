import os
import json
import pandas as pd
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('path', type=str, help='output path')
args = parser.parse_args()

with open('config.json') as infile:
    config = json.load(infile)
    
repo = config['desktop']

cols = ['position', 'method', 'run', 'traj_length', 
        'collisions', 'accept1', 'path1', 'plan1', 'steps1', 
        'latency1', 'accept2', 'path2', 'plan2', 'steps2', 'latency2',
        'state_size', 'n_states', 'adj_list_compile', 'stack_compile',
        'set_compile', 'max_stack_capacity', 'max_set_size', 'virt_min',
        'virt_max', 'res_min', 'res_max', 'shr_min', 'shr_max', 'mem_perc_min', 'mem_perc_max',
        'mem_total_min', 'mem_total_max', 'mem_free_min', 'mem_free_max',
        'mem_used_min', 'mem_used_max', 'cache_min', 'cache_max', 'swap_total_min', 'swap_total_max',
        'swap_free_min', 'swap_free_max', 'swap_used_min', 'swap_used_max', 'process_uptime']

data = []

for run in os.listdir(repo):
    row = []
    with open(repo+'/{}/{}.json'.format(run, run)) as infile:
        d = json.load(infile)
     
    for col in cols:
        row.append(d[col])
        
    data.append(row)
            
df = pd.DataFrame(data, columns=cols);
df.to_csv('DATASET.csv')


        
    
        
    
    