ffmpeg -i Ganglion_how2catchSquirrel_lambda_05X4_deep.mp4 -i Recon_how2catchSquirrel_lambda_05X4_deep.mp4 -i ReconInfra_how2catchSquirrel_lambda_05X4_deep.mp4 -i ReconInfra_Recon_how2catchSquirrel_lambda_05X4_deep.mp4 -filter_complex "nullsrc=size=960x960 [base]; [0:v] setpts=PTS-STARTPTS, scale=480x480 [upperleft]; [1:v] setpts=PTS-STARTPTS, scale=480x480 [upperright]; [2:v] setpts=PTS-STARTPTS, scale=480x480 [lowerleft]; [3:v] setpts=PTS-STARTPTS, scale=480x480 [lowerright]; [base][upperleft] overlay=shortest=1 [tmp1]; [tmp1][upperright] overlay=shortest=1:x=480 [tmp2]; [tmp2][lowerleft] overlay=shortest=1:y=480 [tmp3]; [tmp3][lowerright] overlay=shortest=1:x=480:y=480" -c:v libx264 mosaic_how2catchSquirrel_lambda_05X4_deep.mp4

ffmpeg -i ./output_2013_01_24_how2catchSquirrel_12x12x128_lambda_05X1_deep/Recon/ReconInfra_Recon_how2catchSquirrel_lambda_05X1_deep.mp4 -i ./output_2013_01_24_how2catchSquirrel_12x12x128_lambda_05X2_deep/Recon/ReconInfra_Recon_how2catchSquirrel_lambda_05X2_deep.mp4 -i ./output_2013_01_24_how2catchSquirrel_12x12x128_lambda_05X3_deep/Recon/ReconInfra_Recon_how2catchSquirrel_lambda_05X3_deep.mp4 -i ./output_2013_01_24_how2catchSquirrel_12x12x128_lambda_05X4_deep/Recon/ReconInfra_Recon_how2catchSquirrel_lambda_05X4_deep.mp4 -filter_complex "nullsrc=size=960x960 [base]; [0:v] setpts=PTS-STARTPTS, scale=480x480 [upperleft]; [1:v] setpts=PTS-STARTPTS, scale=480x480 [upperright]; [2:v] setpts=PTS-STARTPTS, scale=480x480 [lowerleft]; [3:v] setpts=PTS-STARTPTS, scale=480x480 [lowerright]; [base][upperleft] overlay=shortest=1 [tmp1]; [tmp1][upperright] overlay=shortest=1:x=480 [tmp2]; [tmp2][lowerleft] overlay=shortest=1:y=480 [tmp3]; [tmp3][lowerright] overlay=shortest=1:x=480:y=480" -c:v libx264 mosaic_ReconInfra_Recon_how2catchSquirrel_deep.mp4


ffmpeg -i ./output_2013_01_24_how2catchSquirrel_12x12x128_lambda_05X2_noPulvinar/Recon/Recon_how2catchSquirrel_lambda_05X2_noPulvinar.mp4 -i ./output_2013_01_24_how2catchSquirrel_12x12x128_lambda_05X2_noise_05_noPulvinar/Recon/Recon_how2catchSquirrel_lambda_05X2_noise_05_noPulvinar.mp4 -i ./output_2013_01_24_how2catchSquirrel_12x12x128_lambda_05X2_noise_10_noPulvinar/Recon/Recon_how2catchSquirrel_lambda_05X2_noise_10_noPulvinar.mp4 -i ./output_2013_01_24_how2catchSquirrel_12x12x128_lambda_05X2_noise_20_noPulvinar/Recon/Recon_how2catchSquirrel_lambda_05X2_noise_20_noPulvinar.mp4 -filter_complex "nullsrc=size=960x960 [base]; [0:v] setpts=PTS-STARTPTS, scale=480x480 [upperleft]; [1:v] setpts=PTS-STARTPTS, scale=480x480 [upperright]; [2:v] setpts=PTS-STARTPTS, scale=480x480 [lowerleft]; [3:v] setpts=PTS-STARTPTS, scale=480x480 [lowerright]; [base][upperleft] overlay=shortest=1 [tmp1]; [tmp1][upperright] overlay=shortest=1:x=480 [tmp2]; [tmp2][lowerleft] overlay=shortest=1:y=480 [tmp3]; [tmp3][lowerright] overlay=shortest=1:x=480:y=480" -c:v libx264 mosaic_Recon_how2catchSquirrel_labmda_05X2_noise_noPulvinar.mp4

ffmpeg -i ./output_2013_01_24_how2catchSquirrel_12x12x128_lambda_05X2_deep/Recon/Ganglion_how2catchSquirrel_lambda_05X2_deep.mp4 -i ./output_2013_01_24_how2catchSquirrel_12x12x128_lambda_05X2_noise_05_deep/Recon/Ganglion_how2catchSquirrel_lambda_05X2_noise_05_deep.mp4 -i ./output_2013_01_24_how2catchSquirrel_12x12x128_lambda_05X2_noise_10_deep/Recon/Ganglion_how2catchSquirrel_lambda_05X2_noise_10_deep.mp4 -i ./output_2013_01_24_how2catchSquirrel_12x12x128_lambda_05X2_noise_20_deep/Recon/Ganglion_how2catchSquirrel_lambda_05X2_noise_20_deep.mp4 -filter_complex "nullsrc=size=960x960 [base]; [0:v] setpts=PTS-STARTPTS, scale=480x480 [upperleft]; [1:v] setpts=PTS-STARTPTS, scale=480x480 [upperright]; [2:v] setpts=PTS-STARTPTS, scale=480x480 [lowerleft]; [3:v] setpts=PTS-STARTPTS, scale=480x480 [lowerright]; [base][upperleft] overlay=shortest=1 [tmp1]; [tmp1][upperright] overlay=shortest=1:x=480 [tmp2]; [tmp2][lowerleft] overlay=shortest=1:y=480 [tmp3]; [tmp3][lowerright] overlay=shortest=1:x=480:y=480" -c:v libx264 mosaic_Ganglion_how2catchSquirrel_labmda_05X2_noise_deep.mp4


ffmpeg -i DCA_ImageDecon.mp4 -i DCA_ImageDeconS1.mp4 -i DCA_ImageDeconS2.mp4 -i DCA_ImageDeconS3.mp4  -filter_complex "nullsrc=size=512x394 [base]; [0:v] setpts=PTS-STARTPTS, scale=256x192 [topleft]; [1:v] setpts=PTS-STARTPTS, scale=256x192 [bottomleft]; [2:v] setpts=PTS-STARTPTS, scale=256x192 [topright]; [3:v] setpts=PTS-STARTPTS, scale=256x192 [bottomright]; [base][topleft] overlay=shortest=1 [tmp1]; [tmp1][bottomleft] overlay=shortest=1:y=192 [tmp2]; [tmp2][topright] overlay=shortest=1:x=256 [tmp3]; [tmp3][bottomright] overlay=shortest=1:x=256:y=192" -c:v libx264 mosaic_ImageDeconDCA.mp4

ffmpeg -i ImageReconS1_dtMax_2.mp4 -i ImageReconS2_dtMax_2.mp4 -filter_complex "nullsrc=size=512x192 [base]; [0:v] setpts=PTS-STARTPTS, scale=256x192 [left]; [1:v] setpts=PTS-STARTPTS, scale=256x192 [right]; [base][left] overlay=shortest=1 [tmp1]; [tmp1][right] overlay=shortest=1:x=256" -c:v libx264 mosaic_1X2_ImageReconS1S2_dtMax_2.mp4