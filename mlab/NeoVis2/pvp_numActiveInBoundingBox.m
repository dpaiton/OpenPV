function [pvp_num_active_BB_mask, ...
	  pvp_num_active_BB_notmask, ...
	  pvp_num_BB_mask, ...
	  pvp_num_BB_notmask] = ...
      pvp_numActiveInBoundingBox(pvp_activity, ...
				 true_CSV_struct);
  global NFEATURES NCOLS NROWS N
  num_BBs = length(true_CSV_struct);
  BB_mask = zeros(NROWS, NCOLS);
  for i_BB = 1 : num_BBs
    x_BB_min = min([true_CSV_struct{i_BB}.BoundingBox_X1; ...
		   true_CSV_struct{i_BB}.BoundingBox_X2; ...
		   true_CSV_struct{i_BB}.BoundingBox_X3; ...
		   true_CSV_struct{i_BB}.BoundingBox_X4]);
    x_BB_max = max([true_CSV_struct{i_BB}.BoundingBox_X1; ...
		   true_CSV_struct{i_BB}.BoundingBox_X2; ...
		   true_CSV_struct{i_BB}.BoundingBox_X3; ...
		   true_CSV_struct{i_BB}.BoundingBox_X4]);
    y_BB_min = min([true_CSV_struct{i_BB}.BoundingBox_Y1; ...
		   true_CSV_struct{i_BB}.BoundingBox_Y2; ...
		   true_CSV_struct{i_BB}.BoundingBox_Y3; ...
		   true_CSV_struct{i_BB}.BoundingBox_Y4]);
    y_BB_max = max([true_CSV_struct{i_BB}.BoundingBox_Y1; ...
		   true_CSV_struct{i_BB}.BoundingBox_Y2; ...
		   true_CSV_struct{i_BB}.BoundingBox_Y3; ...
		   true_CSV_struct{i_BB}.BoundingBox_Y4]);
    BB_mask(y_BB_min:y_BB_max, x_BB_min:x_BB_max) = 1;
  endfor
  pvp_activity3D = reshape(full(pvp_activity), [NFEATURES NCOLS NROWS]);
  pvp_activity3D = permute(pvp_activity3D, [3,2,1]);
  pvp_activity2D = squeeze(sum(pvp_activity3D, 3));
  pvp_num_active_BB_mask = nnz(pvp_activity2D .* (BB_mask==1));
  pvp_num_active_BB_notmask = nnz(pvp_activity2D .* (BB_mask==0));
  pvp_num_BB_mask = nnz(BB_mask);
  pvp_num_BB_notmask = nnz(~BB_mask);

endfunction %% pvp_numActiveInBoundingBoxy