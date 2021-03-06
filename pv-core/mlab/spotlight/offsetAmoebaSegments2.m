function [amoeba_outline_x, amoeba_outline_y, amoeba_struct] = ...
      offsetAmoebaSegments2(amoeba_struct, amoeba_outline_x, amoeba_outline_y)


  %% set location and fix boundaries (use mirror BCs)
  offset_x = 2 * ( rand(1) - 0.5 ) * ( fix(amoeba_struct.image_rect_size(2)/2) - ...
				      amoeba_struct.outer_diameter );
  offset_y = 2 * ( rand(1) - 0.5 ) * ( fix(amoeba_struct.image_rect_size(1)/2) - ...
				      amoeba_struct.outer_diameter );
  amoeba_outline_x = ...
      amoeba_outline_x + fix(amoeba_struct.image_rect_size(2)/2) + offset_x;
  amoeba_outline_x = ...
      amoeba_outline_x .* ...
      ((amoeba_outline_x >= 1) & (amoeba_outline_x <= amoeba_struct.image_rect_size(2))) + ...
      (2 * amoeba_struct.image_rect_size(2) - amoeba_outline_x ) .* ...
      (amoeba_outline_x > amoeba_struct.image_rect_size(2)) + ...
      (1 - amoeba_outline_x) .* (amoeba_outline_x < 1);
  amoeba_outline_y = ...
      amoeba_outline_y + fix(amoeba_struct.image_rect_size(1)/2) + offset_y;
  amoeba_outline_y = ...
      amoeba_outline_y .* ...
      ((amoeba_outline_y >= 1) & (amoeba_outline_y <= amoeba_struct.image_rect_size(1))) + ...
      (2 * amoeba_struct.image_rect_size(1) - amoeba_outline_y ) .* ...
      (amoeba_outline_y > amoeba_struct.image_rect_size(1)) + ...
      (1 - amoeba_outline_y) .* (amoeba_outline_y < 1);
