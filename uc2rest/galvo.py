class Galvo(object):
    def __init__(self, channel=1, frequency=1000, offset=0, amplitude=1/2, clk_div=0):
        '''
        defaults:
            dac->Setup(DAC_CHANNEL_1, 0, 1, 0, 0, 2);
            dac->Setup(dac_channel, clk_div, frequency, scale, phase, invert);
      '''
        self.channel= channel
        self.frequency = frequency
        self.offset = offset
        self.amplitude = amplitude
        self.clk_div = clk_div
        self.path = "/dac_act"

    def return_dict(self):
        dict = {
        "task":self.path,
        "dac_channel": self.channel, # 1 or 2
        "frequency": self.frequency,
        "offset": self.offset,
        "amplitude":self.amplitude,
        "clk_div": self.clk_div
        }
        return dict
