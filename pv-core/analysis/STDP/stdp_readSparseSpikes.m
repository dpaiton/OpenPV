function [spikes, ave_rate] = stdp_readSparseSpikes(fname, begin_step, end_step)

global input_dir n_time_steps 

filename = fname;
filename = [input_dir, filename];
fprintf('read spikes from %s\n',filename);
fprintf('begin_step = %d end_step = %d\n',begin_step, end_step);

debug = 0;  % if 1 prints spiking neurons

ave_rate = 0;
if begin_step > n_time_steps
    begin_step = 1;
end

if exist(filename,'file')
    total_spikes = 0;
    fid = fopen(filename, 'r', 'native');
    [time,numParams,NX,NY,NF] = readHeader(fid);
    fprintf('NX = %d NY = %d NF = %d \n',NX,NY,NF);
    %pause
    
    N = NX * NY * NF;
    minInd = N+1;
    maxInd = -1;
   
    R = [];
    T = [];
    t = 0;
    for i_step = 1 : n_time_steps
        eofstat = feof(fid);
        if (eofstat)
            n_time_steps = i_step - 1;
            fprintf('feof reached: n_time_steps = %d eof = %d\n',...
                n_time_steps,eofstat);
            break;
        else
            eofstat = feof(fid);
            time = fread(fid,1,'float64');
            if isempty(time)
                break
            else
                %fprintf('time = %f\n',time);
            end
            %fprintf('time = %f\n',time);
            num_spikes = fread(fid, 1, 'int');
            %fprintf('time = %f num_spikes = %d ',time, num_spikes);
            %fprintf('eofstat = %d\n', eofstat);
            %pause
        end
        
        S =fread(fid, num_spikes, 'int'); % S is a column vector
        
        if debug 
            fprintf('%d: %f number of spikes = %d: ', ...
                i_step, time, num_spikes);
            for i=1:length(S)
                fprintf('%d ',S(i));
            end
            fprintf('\n');
            %pause
        else
            %fprintf('%d: %f number of spikes = %d eof = %d\n', ...
            %    i_step, time, num_spikes,eofstat);
        end
        
        maxInd = max([maxInd S']);
        minInd = min([minInd S']);
        
        if i_step < begin_step
            continue
        elseif i_step > end_step
             break
        else
            t=t+1;
            T(t) = time;
            R(t) = 2000 * (num_spikes*1.0)/N;
        end
        
        total_spikes = total_spikes + num_spikes;
    end
    fclose(fid);
    figure('Name', 'Average Activity vs Time '); 
    plot(T,R,'or');
    xlabel('time');
    ylabel('average activity');
    
    ave_rate = 1000 * total_spikes / ( N * ( end_step - begin_step + 1 ) * 0.5 );
    fprintf('i_step = %d minInd = %d maxInd = %d aveRate = %f\n',...
        i_step,minInd,maxInd,ave_rate);
    %pause
    
    debug = 0;
    
    %%  Reopen file, read header
    fid = fopen(filename, 'r', 'native');
    tmp = fread(fid, numParams, 'int');
    %begin_step = 40000;
    
    spike_id = [];
    spike_step = [];
    for i_step = 1 : n_time_steps
        
        time = fread(fid,1,'float64');
        if isempty(time)
            break
        else
            %fprintf('time = %f\n',time);
        end
        num_spikes = fread(fid, 1, 'int');
        S = fread(fid, num_spikes, 'int');
        
        if debug
            fprintf('%d: %f number of spikes = %d: ', ...
                i_step, time, num_spikes);
            for i=1:length(S)
                fprintf('%d ',S(i));
            end
            fprintf('\n');
            pause
        end
        
        if i_step < begin_step
            continue
        end
        if i_step > end_step
            break
        end
        spike_id = [spike_id; S+1];
        spike_step = [spike_step; repmat(i_step - begin_step + 1, num_spikes, 1)];
        %pause
         if mod(i_step - begin_step, 1000) == 0
             disp(['i_step = ', num2str(i_step)]);
         end
    end
    fclose(fid);
    spikes = sparse(spike_step, spike_id, 1, end_step - begin_step + 1, N, total_spikes);
    ave_rate = 1000 * sum(spikes(:)) / ( N * ( end_step - begin_step + 1 ) * 0.5);
    disp(['ave_rate = ', num2str(ave_rate)]);
    size(spikes)
else
    disp(['Skipping, could not open ', filename]);
    spikes = sparse([], [], [], end_step - begin_step + 1, N, 0);
    ave_rate = 0; 
end

%
% end primary function
    
    
function [time,numParams,NX,NY,NF] = readHeader(fid)
% NOTE: see analysis/python/PVReadWeights.py for reading params
% We call this function first because it rewinds the input file

    head = fread(fid,3,'int');
    if head(3) ~= 2  % PV_INT_TYPE
       disp('incorrect file type')
       return
    end
    numParams = head(2);
    fseek(fid,0,'bof'); % rewind file
    params = fread(fid, numParams-2, 'int'); 
    %pause
    NX         = params(4);
    NY         = params(5);
    NF         = params(6);
    fprintf('numParams = %d ',numParams);
    fprintf('NX = %d NY = %d NF = %d ',NX,NY,NF);
    %pause
    % read time - last two params
    time = fread(fid,1,'float64');
    fprintf('time = %f\n',time);
    %pause
    
% End subfunction 
%    
