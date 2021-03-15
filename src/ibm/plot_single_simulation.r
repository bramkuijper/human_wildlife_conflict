#!/usr/bin/env Rscript 

#--vanilla

library("ggplot2")
library("gridExtra")

# get command line arguments
args = commandArgs(trailingOnly=TRUE)

# give an error message if you do not provide it with a simulation file name
if (length(args) < 1)
{
    print("provide a simulation file name")
    stop()
}

# find out where the parameter listing starts
# so that we can read in the data part of the file 
# without having it messed up by the subsequent parameter listing
find_out_param_line <- function(filename) {

    f <- readLines(filename)

    # make a reverse sequence
    seqq <- seq(length(f),1,-1)

    # go through each line in the data file and find first line
    # where data is printed (i.e., a line which starts with a digit)
    for (line_i in seqq)
    {
        print(f[[line_i]])
        print(line_i)
        if (length(grep("^\\d",f[[line_i]])) > 0)
        {
            return(line_i)
        }
    }

    return(NA)
}

parameter_row <- find_out_param_line(args[1])

if (is.na(parameter_row))
{
    print("cannot find data...")
    stop()
}

# read in data frame of corresponding simulation
the.data <- read.table(args[1], header=T, nrow=parameter_row - 1, sep=";")

# now use ggplot2 to plot stuff

str(the.data)

resource <- ggplot(data=the.data
        ,aes(x=generation)) +
            geom_line(aes(y = animal_resource),color="blue") +
            geom_line(aes(y = animal_resource + sqrt(var_animal_resource/500))
                    ,color="skyblue1") +
            geom_line(aes(y = animal_resource - sqrt(var_animal_resource/500))
                    ,color="skyblue1") +
            theme_classic() + 
            xlab("Generation") + 
            ylab("Resource")

colors <- c("Site 0" = "red"
        ,"Site 1" = "blue"
        ,"Site 2" = "orange")

sites <- ggplot(data=the.data
        ,aes(x=generation)) +
            geom_line(aes(y = n_site0, color="Site 0")) +
            geom_line(aes(y = n_site1, color="Site 1")) +
            geom_line(aes(y = n_site2, color="Site 2")) +
            theme_classic() +
            labs(x =""
                    ,y="N sites"
                    ,color="Legend") +
            scale_color_manual(values=colors)

colors <- c("Site 0" = "red"
        ,"Site 1" = "blue"
        ,"Site 2" = "orange")

resource_site <- ggplot(data=the.data
        ,aes(x=generation)) +
            geom_line(aes(y = resource_site_type0, color="Site 0")) +
            geom_line(aes(y = resource_site_type1, color="Site 1")) +
            geom_line(aes(y = resource_site_type2, color="Site 2")) +
            theme_classic() + 
            labs(x =""
                    ,y="Resources/site"
                    ,color="Legend") +
            scale_color_manual(values=colors)

colors <- c("Site 0" = "red"
        ,"Site 1" = "blue"
        ,"Site 2" = "orange")

attack_rates <- ggplot(data=the.data
        ,aes(x=generation)) +
            geom_line(aes(y = attack_rate0, color="Site 0")) +
            geom_line(aes(y = attack_rate1, color="Site 1")) +
            geom_line(aes(y = attack_rate2, color="Site 2")) +
            theme_classic() + 
            labs(x =""
                    ,y="Attack"
                    ,color="Legend") +
            scale_color_manual(values=colors)


big_plot <- arrangeGrob(resource, sites, resource_site, attack_rates, nrow=4,ncol=1)
the.base.name <- basename(args[1])

output_file_name <- paste(
        "graph_"
        ,the.base.name
        ,".pdf"
        ,sep="")

ggsave(output_file_name, big_plot, height = 15)

