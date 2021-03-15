library("simulation.utils")

params <- list()

params$p1 <- 0.5
params$p2 <- 0.25
params$p3 <- 0.25
params$init_animal_resources <- 1.0
params$init_site_resources <- 1.0


# expand all parameter combinations to one data frame
all_params = as.data.frame(expand.grid(params))

# make the batch file
make.batch.file(
  parameter_list = all_params
  ,executable_path = "./hwc.exe"
  ,n_replicates = 3
)

