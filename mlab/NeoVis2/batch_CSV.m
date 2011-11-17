
object_ids = [16]; %% [7:17,21:22,30:31];
object_name = cell(length(object_ids),1);
for i_object = 1 : (length(object_name)-1)
                 object_name{i_object} = num2str(object_ids(i_object), "%3.3i");
endfor
object_name{length(object_name)} = num2str(object_ids(length(object_name)), "%3.3i");

for i_object = 1 : length(object_name)
disp(object_name{i_object});
pvp_makeCSVFile([], ...
                [], ...
                [], ...
                [], ...
                object_name{i_object}, ...
                [], ...
                [], ...
                [], ...
                [], ...
                [], ...
                [], ...
                [], ...
                [], ...
                []);
csv_file = ...
  ["/mnt/data1/repo/neovision-programs-petavision/Tailwind/Challenge/results/", object_name{i_object}, "/Car3/canny/", "Tailwind_", object_name{i_object}, "_PetaVision_", "Car", "_000", ".csv"]
csv_repo = ...
  ["/mnt/data1/repo/neovision-results-challenge-tailwind/", object_name{i_object}, filesep]
mkdir(csv_repo);
copyfile(csv_file, csv_repo)
endfor



