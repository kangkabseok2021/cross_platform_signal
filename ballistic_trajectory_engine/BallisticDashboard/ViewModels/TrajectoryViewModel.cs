using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using BallisticDashboard.Interop;
using BallisticDashboard.Models;
using OxyPlot;
using OxyPlot.Annotations;
using OxyPlot.Axes;
using OxyPlot.Series;

namespace BallisticDashboard.ViewModels;

public sealed class TrajectoryViewModel : INotifyPropertyChanged
{
    private readonly NativeBallisticsClient _client = new();

    private double      _elevationDeg      = 45.0;
    private double      _muzzleVelocityMs  = 400.0;
    private MunitionKind _selectedMunition = MunitionKind.Artillery155mm;
    private string      _statusText        = "Set parameters and click Compute.";
    private PlotModel   _plotModel;

    public TrajectoryViewModel()
    {
        _plotModel     = BuildEmptyPlot();
        ComputeCommand = new RelayCommand(_ => _ = ExecuteComputeAsync());
    }

    public ICommand ComputeCommand { get; }

    public IEnumerable<MunitionKind> MunitionTypes => Enum.GetValues<MunitionKind>();

    public double ElevationDeg
    {
        get => _elevationDeg;
        set { _elevationDeg = Math.Clamp(value, 1.0, 89.0); OnPropertyChanged(); OnPropertyChanged(nameof(ElevationLabel)); }
    }

    public double MuzzleVelocityMs
    {
        get => _muzzleVelocityMs;
        set { _muzzleVelocityMs = Math.Clamp(value, 100.0, 900.0); OnPropertyChanged(); OnPropertyChanged(nameof(MuzzleVelocityLabel)); }
    }

    public MunitionKind SelectedMunitionType
    {
        get => _selectedMunition;
        set { _selectedMunition = value; OnPropertyChanged(); }
    }

    public string ElevationLabel      => $"Elevation: {_elevationDeg:F1}°";
    public string MuzzleVelocityLabel => $"Muzzle Velocity: {_muzzleVelocityMs:F0} m/s";

    public string StatusText
    {
        get => _statusText;
        private set { _statusText = value; OnPropertyChanged(); }
    }

    public PlotModel PlotModel
    {
        get => _plotModel;
        private set { _plotModel = value; OnPropertyChanged(); }
    }

    private async Task ExecuteComputeAsync()
    {
        StatusText = "Computing…";
        try
        {
            var pts = await Task.Run(() =>
                _client.Compute(_elevationDeg, _muzzleVelocityMs, _selectedMunition));

            await Application.Current.Dispatcher.InvokeAsync(() => UpdatePlot(pts));
        }
        catch (BallisticsException ex)
        {
            StatusText = $"Error: {ex.Message}";
        }
    }

    private void UpdatePlot(IReadOnlyList<TrajectoryPoint> pts)
    {
        var model = BuildEmptyPlot();
        var series = new LineSeries
        {
            Title       = _selectedMunition.ToString(),
            Color       = MunitionColor(_selectedMunition),
            StrokeThickness = 2
        };
        foreach (var p in pts)
            series.Points.Add(new DataPoint(p.XMeters, p.YMeters));
        model.Series.Add(series);

        // Apex annotation
        var apex = pts.MaxBy(p => p.YMeters)!;
        model.Annotations.Add(new TextAnnotation
        {
            Text            = $"Apex {apex.YMeters:F0} m",
            TextPosition    = new DataPoint(apex.XMeters, apex.YMeters),
            FontSize        = 10,
            TextColor       = OxyColors.Gray
        });

        // Landing annotation
        var land = pts.Last();
        model.Annotations.Add(new TextAnnotation
        {
            Text            = $"Range {land.XMeters:F0} m",
            TextPosition    = new DataPoint(land.XMeters, 0),
            FontSize        = 10,
            TextColor       = OxyColors.Gray
        });

        model.InvalidatePlot(true);
        PlotModel  = model;
        StatusText = $"Range: {land.XMeters:F0} m | Apex: {apex.YMeters:F0} m | ToF: {land.TimeS:F1} s";
    }

    private static PlotModel BuildEmptyPlot()
    {
        var m = new PlotModel { Title = "Ballistic Trajectory" };
        m.Axes.Add(new LinearAxis { Position = AxisPosition.Bottom, Title = "Range (m)", Minimum = 0 });
        m.Axes.Add(new LinearAxis { Position = AxisPosition.Left,   Title = "Altitude (m)", Minimum = 0 });
        return m;
    }

    private static OxyColor MunitionColor(MunitionKind k) => k switch
    {
        MunitionKind.Artillery155mm => OxyColors.SteelBlue,
        MunitionKind.Mortar81mm     => OxyColors.ForestGreen,
        MunitionKind.APFSDS120mm    => OxyColors.OrangeRed,
        _                           => OxyColors.Gray
    };

    public event PropertyChangedEventHandler? PropertyChanged;
    private void OnPropertyChanged([CallerMemberName] string? name = null)
        => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
}
