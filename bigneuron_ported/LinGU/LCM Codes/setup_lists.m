function [params] = setup_lists(params)

% Load the file lists containing GT, images, masks, ...
%
%  authors: Carlos Becker, Roberto Rigamonti, CVLab EPFL
%  e-mail: name <dot> surname <at> epfl <dot> ch
%  web: http://cvlab.epfl.ch/
%  date: February 2014

for i_op = 1:params.ops_no
    op = params.ops{i_op};

    params.gts.(op).list_filename = fullfile('lists',params.dataset_name,sprintf('%s_gt.txt',op));
    [params.gts.(op).list,params.gts.(op).imgs_no] = get_list(params.gts.(op).list_filename);
    params.masks.(op).list_filename = fullfile('lists',params.dataset_name,sprintf('%s_masks.txt',op));
    [params.masks.(op).list,params.masks.(op).imgs_no] = get_list(params.masks.(op).list_filename);

    %% Get the lists
    for i_ch = 1:params.ch_no
        ch = params.ch_list{i_ch};
        params.(ch).(op).list_filename = fullfile('lists',params.dataset_name,sprintf('%s_%s.txt',op,ch));
        [params.(ch).(op).list,params.(ch).(op).imgs_no] = get_list(params.(ch).(op).list_filename);
    end

    %% Set the file list
    params.(op).input_file_list = params.(params.ch_list{1}).(op).list;
    
    %% Validate the lists
    ch = params.ch_list{1};
    imgs_no = params.(ch).(op).imgs_no;
    fprintf('  -- %d images in %s set --\n',imgs_no,op);
    if (imgs_no~=params.gts.(op).imgs_no)
        error('Invalid GTs list, operation %s (size is %d, should have been %d)',op,params.gts.(op).imgs_no,imgs_no);
    end
    if (imgs_no~=params.masks.(op).imgs_no)
        error('Invalid masks list, operation %s (size is %d, should have been %d)',op,params.masks.(op).imgs_no,imgs_no);
    end
end

end
