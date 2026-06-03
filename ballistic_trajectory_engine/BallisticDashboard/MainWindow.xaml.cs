using System.Windows;
using BallisticDashboard.ViewModels;

namespace BallisticDashboard;

public partial class MainWindow : Window
{
    public MainWindow()
    {
        InitializeComponent();
        DataContext = new TrajectoryViewModel();
    }
}
