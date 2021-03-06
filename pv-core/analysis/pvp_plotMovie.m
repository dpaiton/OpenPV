function [fig_h] = ...
      pvp_plotMovie(layer, ...
		    epoch_struct, ...
		    layer_struct, ...
		    target_struct, ...
		    movie_epoch,
		    MOVIE_PATH)

  global BIN_STEP_SIZE DELTA_T
  global pvp_order
  fig_list = [];

  %% start loop over epochs
  for i_epoch = 1 : epoch_struct.num_epochs
    disp(["i_epoch = ", num2str(i_epoch)]);
    
    %% movie plot
    if  ~ismember( i_epoch, movie_epoch )
      continue;
    endif %%

    %% Rea spike train for this epoch
    [spike_array] = ...
        pvp_readSparseSpikes(layer, ...
			     i_epoch, ...
			     epoch_struct, ...
			     layer_struct, ...
			     pvp_order);
    if isempty(spike_array)
      continue;
    endif %%
    ave_image = zeros(layer_struct.num_rows(layer), layer_struct.num_cols(layer));
    
    [num_steps, num_neurons] = size(spike_array);
    frame_duration = 12;
    for i_step = 1 : num_steps-frame_duration

      plot_title = ...
          [layer_struct.layerID{layer}, ...
           "_", ...
           int2str(layer), ...
           "_", ...
           int2str(i_epoch), ...
           "_", ...
	   num2str(i_step, "%5.5d")];
      plot_title(plot_title==" ") = "";
      spike_image = zeros(layer_struct.num_rows(layer), layer_struct.num_cols(layer));
      spike3D_ndx = find(any(spike_array(i_step:i_step+frame_duration,:),1));
      [spike_feature_ndx, spike_col_ndx, spike_row_ndx] = ...
	  ind2sub([layer_struct.num_features(layer), ...
		   layer_struct.num_cols(layer), ...
		   layer_struct.num_rows(layer)], ...
		  spike3D_ndx);
      spike2D_ndx = ...
	  sub2ind([layer_struct.num_rows(layer), ...
		   layer_struct.num_cols(layer)], ...
		  spike_row_ndx, ...
		  spike_col_ndx);
      spike_image(spike2D_ndx) = 255;
      %%spike_image = flipud(spike_image);
      imwrite(uint8(spike_image), [MOVIE_PATH, filesep, plot_title, ".png"]);
      ave_image = ave_image + spike_image;
    endfor  %% i_step
    plot_title = ...
        [layer_struct.layerID{layer}, ...
         " Movie",...
         "_", ...
         int2str(layer), ...
         "_", ...
         int2str(i_epoch)];
    plot_title(plot_title==" ") = "";
    fig_h = figure("Name", plot_title);
    axis([1 layer_struct.num_rows(layer) 1 layer_struct.num_cols(layer) ]);
    axis "tight";
    axis "image";
    axis "ij"
    %%axis normal
    imagesc(ave_image);
    box off
    axis off
  endfor %% i_epoch
