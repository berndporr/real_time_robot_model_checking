Supporting code for the case study in Chapter 4.   

Data is available at https://doi.org/10.5525/gla.researchdata.2012

# cul-de-sac

This folder contains analysis scripts and jupyter notebooks for the cul-de-sac scenario.  The analysis assumes that there is a repository of raw data in some directory `repo` and outputs all interim files to the repository during cleaning.  



## Trajectory preparation

First we run `python robotracker.py repo/run/run` to extract the trajectory for a run using the optical flow method in OpenCV.  Note that the path to the run video excludes the file extension `.mp4`.  As the optical flow method can be sensitive to light conditions, the process for extracting and cleaning trajectories for analysis is semi-automatic.   Visual inspection is necessary for precision.  

Running the `robotracker.py` script outputs three files into the directory `repo/run`: 

- `run_coord.txt`
- `run_trace.png`
- `run_1stframe.png`

Once these files have been generated, it is then possible to load the trajectory using `clean_traj.ipynb`, manually inspect and clean it then write the final trajectory to `repo/run/run_coord.csv`.  Cleaning involves removing additional points that may have been picked up by the optical flow method in OpenCV and truncating so all trajectories start and end at the same entrance to the cul-de-sace.  We also specify the number of collisions observed and add this to run metadata in `repo/run/run.json`. This is repeated for each run. 

## Extracting and cleaning log data

Once the trajectories have been extracted, we then use `clean_logs.ipynb` to extract relevant data and add it to `repo/run/run.json`.  Specifically, this extracts plan data, model checking memory usage, and process memory useage. Note that this is fully automated.  

## Building the dataset

Once the above steps have been done, we then run `python build_dataset.py` to iterate through all runs in `repo` and build a dataset, output as a `.csv` file. We have included `DATASET.csv` which is the dataset we used for analysis.  

## Running analysis

It is then possible to run `analysis.ipynb` to generate results.  

# playground

This folder contains analysis scripts and jupyter notebooks for the playground scenario, which again assumes there is a repository of raw data in some directory `repo`.  

Data is available at: TODO!

### Trajectory preparation

First we run `python robotracker.py repo/run/run` to extract the trajectory for a run using the optical flow method in OpenCV, as for the cul-de-sac scenario.  In this case, we are interested in `run_trace.png` as a visual representation of the trajectories for comparison.  This aspect is again done manually, however there are only two comparisons in this case.  We also view the video of the run `repo/run/run.mp4`, manually count the number of times the cul-de-sac feature is visited and time spent inside it.  

### Extracting and cleaning log data

As above, we extract and clean log data using `clean_logs.ipynb` and generate three files:

- `repo/run/runmodel_check.csv` : model checking memory usage
- `repo/run/runusage.csv` : process memory usage (model checking and baseline)
- `repo/run/runplan.csv` :  model checking plan data

### Running analysis

As there are only two comparisons, we run `analysis.ipynb` using the data directly from `repo`. 
  





