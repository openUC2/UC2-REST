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
                       frame_count=0, bidirectional=False, timeout=1):
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
                "bidirectional": 1 if bidirectional else 0
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

