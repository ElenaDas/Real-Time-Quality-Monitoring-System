% Load data from the CSV file
% The readtable function reads the CSV file and loads its contents into a table.
% Each column in the CSV becomes a variable in the table, allowing easy access and manipulation.
data = readtable('sensor_data.csv');

% Convert the Timestamp column to datetime format
% The Timestamp column is initially stored as a string. 
% Here, it is converted into a datetime object to enable time-based plotting and analysis.
data.Timestamp = datetime(data.Timestamp, 'InputFormat', 'yyyy-MM-dd HH:mm:ss');

% Automatically detect unique sensor types
% The unique function retrieves all distinct SensorID values present in the data.
sensorTypes = unique(data.SensorID);

% Determine the number of sensors
% The number of unique SensorIDs determines how many plots we will generate.
numSensors = length(sensorTypes);

% Create a new figure to display all plots
% The figure function initializes a window for displaying the plots.
figure;

% Loop through each sensor type to create individual plots
for i = 1:numSensors
    % Filter the data for the current sensor type
    % This selects all rows where SensorID matches the current sensor type.
    sensorData = data(strcmp(data.SensorID, sensorTypes{i}), :);

    % Create a subplot for the current sensor
    % The subplot function divides the figure into a grid and activates the i-th plot.
    subplot(numSensors, 1, i);
    
    % Plot the sensor values over time
    % The plot function generates a line plot with markers for each data point.
    plot(sensorData.Timestamp, sensorData.Value, '-o');
    
    % Add title and labels to the plot
    % The title, xlabel, and ylabel functions are used to label the plot appropriately.
    title([sensorTypes{i}, ' Over Time']); % Dynamic title based on SensorID
    xlabel('Timestamp');                   % Label for the x-axis
    ylabel('Value');                       % Label for the y-axis (adjusted dynamically)
    
    % Enable grid lines for better readability
    grid on;

    % Calculate and display the average value for the sensor
    % The mean function computes the average of the Value column.
    meanValue = mean(sensorData.Value);
    
    % Add a horizontal line for the average value
    % The yline function draws a horizontal line across the plot.
    hold on;
    yline(meanValue, '--r', ['Average: ', num2str(meanValue)]); % Red dashed line
    hold off;

    % Detect and display outliers
    % Outliers are identified as values outside a predefined acceptable range.
    % For demonstration, we assume TEMP has a valid range of [5, 25].
    if strcmp(sensorTypes{i}, 'TEMP')
        lowerLimit = 5; % Example lower limit for TEMP
        upperLimit = 25; % Example upper limit for TEMP
        outliers = sensorData.Value < lowerLimit | sensorData.Value > upperLimit;
        
        % Highlight outliers in the console
        if any(outliers)
            disp(['Outliers detected for ', sensorTypes{i}, ':']);
            disp(sensorData(outliers, :)); % Display rows with outliers
        end
    end
end

% Save the figure as an image
% The saveas function saves the current figure to a file.
% This is useful for including the plots in reports or sharing results.
saveas(gcf, 'sensor_plots.png');

% End of script
disp('Data analysis and visualization completed successfully.');

