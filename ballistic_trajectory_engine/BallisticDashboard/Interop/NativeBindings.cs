using System;
using System.Runtime.InteropServices;

namespace BallisticDashboard.Interop;

[StructLayout(LayoutKind.Sequential)]
public struct TrajectoryPointNative
{
    public double TimeS;
    public double XMeters;
    public double YMeters;
    public double SpeedMs;
}

internal static class NativeBindings
{
    static NativeBindings()
    {
        // Allow CI to pre-load the DLL from an explicit path before DllImport resolves it.
        var path = Environment.GetEnvironmentVariable("BALLISTIC_DLL_PATH");
        if (!string.IsNullOrEmpty(path))
            NativeLibrary.Load(path);
    }

    [DllImport("ballistic_engine", CallingConvention = CallingConvention.Cdecl)]
    public static extern int compute_trajectory(
        double elevation_deg,
        double muzzle_velocity_ms,
        int munition_id,
        [Out, MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 4)]
        TrajectoryPointNative[] out_points,
        int max_points,
        out int out_count);

    [DllImport("ballistic_engine", CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr get_last_error();
}
