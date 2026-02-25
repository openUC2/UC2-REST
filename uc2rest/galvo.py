import numpy as np
class Galvo(object):
    def __init__(self, parent=None):
        '''
        defaults:
            dac->Setup(DAC_CHANNEL_1, 0, 1, 0, 0, 2);
            dac->Setup(dac_channel, clk_div, frequency, scale, phase, invert);
      '''
        self.channel= 1
        self.frequency = 1
        self.offset = 0
        self.amplitude = amplitude=1/2
        self.clk_div = 0
        self.path = "/dac_act"
        
        self._parent = parent

    
    def set_dac(self, channel=1, frequency=1, offset=0, amplitude=1, clk_div=0, phase=0, invert=1, timeout=1):
        # {"task":"/dac_act", "dac_channel":1, "frequency":10, "offset":1, "amplitude":1, "divider":0, "phase":0, "invert":1, "qid":2}
        path = "/dac_act"
        payload = {
            "task": path,
            "dac_channel": channel, # 1 or 2
            "frequency": frequency,
            "offset": offset,
            "divider": clk_div,
            "amplitude":amplitude, 
            "phase":phase,
            "invert":invert
        }
                        
        self._parent.post_json(path, payload, timeout=timeout, getReturn=False)

    '''
    ##############################################################################################################################
    SCANNER
    ##############################################################################################################################
    '''
    def set_galvo_scan(self, nx=256, ny=256, x_min=500, x_max=3500, 
                       y_min=500, y_max=3500, sample_period_us=1, 
                       frame_count=0, bidirectional=False, 
                       pre_samples=0, fly_samples=0, trig_delay_us=0, 
                       trig_width_us=0, line_settle_samples=0, 
                       enable_trigger=1, apply_x_lut=0,
                       timeout=1):
        """
        Start galvo scanner with new API (HighSpeedScannerCore)
        
        Args:
            nx: Number of X samples per line (default: 256)
            ny: Number of Y lines (default: 256)
            x_min: Min X position 0-4095 (default: 500)
            x_max: Max X position 0-4095 (default: 3500)
            y_min: Min Y position 0-4095 (default: 500)
            y_max: Max Y position 0-4095 (default: 3500)
            sample_period_us: Microseconds per sample, 0=max speed (default: 1)
            frame_count: Number of frames, 0=infinite (default: 0)
            bidirectional: Enable bidirectional scanning (default: False)
            timeout: Request timeout in seconds (default: 1)
            
            sends:
            {"task": "/galvo_act", "config": {"nx":512,"ny":512,"x_min":500,"x_max":3500,"y_min":500,"y_max":3500,"pre_samples":0,"fly_samples":0,"sample_period_us":0,"trig_delay_us":0,"trig_width_us":0,"line_settle_samples":0,"enable_trigger":1,"apply_x_lut":0,"frame_count":0,"bidirectional":true}}
            
        Example:
            >>> galvo.set_galvo_scan(nx=64, ny=64, frame_count=10, bidirectional=True)
        """
        path = '/galvo_act'
        payload = {
            "task": path,
            "config": {
                "nx": nx,
                "ny": ny,
                "x_min": x_min,
                "x_max": x_max,
                "y_min": y_min,
                "y_max": y_max,
                "sample_period_us": sample_period_us,
                "frame_count": frame_count,
                "bidirectional": True if bidirectional else False,
                "pre_samples": pre_samples,
                "fly_samples": fly_samples,
                "trig_delay_us": trig_delay_us,
                "trig_width_us": trig_width_us,
                "line_settle_samples": line_settle_samples,
                "enable_trigger": enable_trigger,
                "apply_x_lut": apply_x_lut
            }
        }
        
        return self._parent.post_json(path, payload, timeout=timeout)
    
    def stop_galvo_scan(self, timeout=1):
        """
        Stop galvo scanner
        
        Args:
            timeout: Request timeout in seconds (default: 1)
            
        Example:
            >>> galvo.stop_galvo_scan()
        """
        path = '/galvo_act'
        payload = {
            "task": path,
            "stop": True
        }
        
        return self._parent.post_json(path, payload, timeout=timeout)
    
    def get_galvo_status(self, timeout=1):
        """
        Get galvo scanner status
        
        Args:
            timeout: Request timeout in seconds (default: 1)
            
        Returns:
            dict: Status including running, current_frame, current_line, config, etc.
            
        Example:
            >>> status = galvo.get_galvo_status()
            >>> print(f"Running: {status['running']}, Frame: {status['current_frame']}")
        """
        path = '/galvo_get'
        payload = {
            "task": path
        }
        
        return self._parent.post_json(path, payload, timeout=timeout)

    def set_arbitrary_points(self, points, laser_trigger="AUTO", timeout=1):
        """
        Start arbitrary point scanning with custom XY coordinate list
        
        Each point in the list specifies an X, Y coordinate, a dwell time,
        and optionally a laser intensity.
        The scanner will loop through the points continuously.
        
        Args:
            points: List of dicts with keys:
                    - 'x' (0-4095): X DAC coordinate
                    - 'y' (0-4095): Y DAC coordinate
                    - 'dwell_us' (int): Dwell time in microseconds
                    - 'laser_intensity' (0-255, optional): Per-point laser intensity
                    Maximum 265 points.
            laser_trigger: Trigger mode - "AUTO" (HIGH during dwell, LOW during movement),
                          "HIGH" (force always on), "LOW" (force always off), 
                          "CONTINUOUS" (HIGH during entire scan)
            timeout: Request timeout in seconds (default: 1)
            
        Example:
            >>> points = [
            ...     {"x": 1024, "y": 2048, "dwell_us": 500, "laser_intensity": 128},
            ...     {"x": 1500, "y": 2100, "dwell_us": 1000, "laser_intensity": 255},
            ...     {"x": 2000, "y": 2500, "dwell_us": 250}
            ... ]
            >>> galvo.set_arbitrary_points(points)
            >>> galvo.set_arbitrary_points(points, laser_trigger="CONTINUOUS")
        """
        if len(points) > 265:
            raise ValueError("Maximum 265 points supported")
        if len(points) == 0:
            raise ValueError("At least 1 point required")
        
        # Validate points
        for i, pt in enumerate(points):
            if not all(k in pt for k in ('x', 'y', 'dwell_us')):
                raise ValueError(f"Point {i} missing required keys (x, y, dwell_us)")
            if pt['x'] < 0 or pt['x'] > 4095:
                raise ValueError(f"Point {i} x={pt['x']} out of range (0-4095)")
            if pt['y'] < 0 or pt['y'] > 4095:
                raise ValueError(f"Point {i} y={pt['y']} out of range (0-4095)")
            if 'laser_intensity' in pt:
                if pt['laser_intensity'] < 0 or pt['laser_intensity'] > 255:
                    raise ValueError(f"Point {i} laser_intensity={pt['laser_intensity']} out of range (0-255)")
        
        path = '/galvo_act'
        payload = {
            "task": path,
            #"laser_trigger": laser_trigger,
            "points": points
        }
        
        return self._parent.post_json(path, payload, timeout=timeout)

    def stop_arbitrary_points(self, timeout=1):
        """
        Stop arbitrary point scanning
        
        Args:
            timeout: Request timeout in seconds (default: 1)
            
        Example:
            >>> galvo.stop_arbitrary_points()
        """
        path = '/galvo_act'
        payload = {"task":"/galvo_act", "points":[{"x":0,"y":0,"dwell_us":1000}]} # Single point at origin with long dwell to effectively stop scanning

        return self._parent.post_json(path, payload, timeout=timeout)

    def pause_arbitrary_points(self, timeout=1):
        """
        Pause arbitrary point scanning (keeps current index)
        
        Args:
            timeout: Request timeout in seconds (default: 1)
            
        Example:
            >>> galvo.pause_arbitrary_points()
        """
        path = '/galvo_act'
        payload = {
            "task": path,
            "pause_points": True
        }
        
        return self._parent.post_json(path, payload, timeout=timeout)

    def resume_arbitrary_points(self, timeout=1):
        """
        Resume arbitrary point scanning from paused position
        
        Args:
            timeout: Request timeout in seconds (default: 1)
            
        Example:
            >>> galvo.resume_arbitrary_points()
        """
        path = '/galvo_act'
        payload = {
            "task": path,
            "resume_points": True
        }
        
        return self._parent.post_json(path, payload, timeout=timeout)
    
    def set_trigger_mode(self, mode="AUTO", timeout=1):
        """
        Set laser trigger mode override
        
        Args:
            mode: Trigger mode - "AUTO", "HIGH", "LOW", "CONTINUOUS"
            timeout: Request timeout in seconds (default: 1)
            
        Example:
            >>> galvo.set_trigger_mode("HIGH")   # Force laser on
            >>> galvo.set_trigger_mode("LOW")    # Force laser off
            >>> galvo.set_trigger_mode("AUTO")   # Restore automatic control
        """
        path = '/galvo_act'
        payload = {
            "task": path,
            "laser_trigger": mode
        }
        
        return self._parent.post_json(path, payload, timeout=timeout)

    def generate_circle_points(self, center_x=2048, center_y=2048, radius=1000, 
                                num_points=32, dwell_us=100):
        """
        Generate points along a circle for arbitrary point scanning
        
        Args:
            center_x: Circle center X (default: 2048)
            center_y: Circle center Y (default: 2048)
            radius: Circle radius in DAC units (default: 1000)
            num_points: Number of points around circle (default: 32)
            dwell_us: Dwell time per point in microseconds (default: 100)
            
        Returns:
            list: List of point dicts suitable for set_arbitrary_points()
            
        Example:
            >>> points = galvo.generate_circle_points(radius=500, num_points=64)
            >>> galvo.set_arbitrary_points(points)
        """
        points = []
        for i in range(num_points):
            angle = 2.0 * np.pi * i / num_points
            x = int(center_x + radius * np.cos(angle))
            y = int(center_y + radius * np.sin(angle))
            x = max(0, min(4095, x))
            y = max(0, min(4095, y))
            points.append({"x": x, "y": y, "dwell_us": dwell_us})
        return points

    def generate_grid_points(self, x_min=500, x_max=3500, y_min=500, y_max=3500,
                              nx=4, ny=4, dwell_us=500):
        """
        Generate a grid of points for arbitrary point scanning
        
        Args:
            x_min, x_max: X range (default: 500-3500)
            y_min, y_max: Y range (default: 500-3500)
            nx, ny: Number of grid points per axis (default: 4x4)
            dwell_us: Dwell time per point in microseconds (default: 500)
            
        Returns:
            list: List of point dicts suitable for set_arbitrary_points()
            
        Example:
            >>> points = galvo.generate_grid_points(nx=8, ny=8, dwell_us=1000)
            >>> galvo.set_arbitrary_points(points)
        """
        points = []
        for iy in range(ny):
            y = int(y_min + (y_max - y_min) * iy / max(1, ny - 1)) if ny > 1 else (y_min + y_max) // 2
            for ix in range(nx):
                x = int(x_min + (x_max - x_min) * ix / max(1, nx - 1)) if nx > 1 else (x_min + x_max) // 2
                points.append({"x": x, "y": y, "dwell_us": dwell_us})
        return points
