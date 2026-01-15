"""
Motor configuration data structures for unified motor parameter management.
This module provides dataclasses for storing motor settings per axis,
including motion parameters, homing configuration, and TMC driver settings.
"""

import json
from dataclasses import dataclass, field, asdict
from typing import Optional, Dict, List


@dataclass
class TMCSettings:
    """TMC stepper driver settings for a single axis."""
    msteps: int = 16  # Microsteps (1, 2, 4, 8, 16, 32, 64, 128, 256)
    rms_current: int = 500  # RMS current in mA
    sgthrs: int = 10  # StallGuard threshold
    semin: int = 5  # Minimum coolstep current
    semax: int = 2  # Maximum coolstep current
    blank_time: int = 24  # Comparator blank time
    toff: int = 3  # Off time

    def to_dict(self) -> dict:
        return asdict(self)

    @classmethod
    def from_dict(cls, data: dict) -> 'TMCSettings':
        if data is None:
            return cls()
        return cls(**{k: v for k, v in data.items() if k in cls.__dataclass_fields__})


@dataclass
class HomingSettings:
    """Homing configuration for a single axis."""
    enabled: bool = False  # Is homing enabled for this axis
    speed: int = 15000  # Homing speed in steps/s
    direction: int = -1  # Homing direction (-1 or 1)
    endstop_polarity: int = 1  # Endstop polarity (0=NO, 1=NC)
    endpos_release: int = 3000  # Steps to back off after hitting endstop
    timeout: int = 20000  # Homing timeout in ms
    home_on_start: bool = False  # Home this axis on startup
    home_steps: int = 0  # Steps to move if no endstop (open-loop homing)

    def to_dict(self) -> dict:
        return asdict(self)

    @classmethod
    def from_dict(cls, data: dict) -> 'HomingSettings':
        if data is None:
            return cls()
        return cls(**{k: v for k, v in data.items() if k in cls.__dataclass_fields__})


@dataclass
class MotionSettings:
    """Motion parameters for a single axis."""
    step_size: float = 1.0  # Steps per µm (calibrated stepsize)
    min_pos: float = float('-inf')  # Minimum position limit
    max_pos: float = float('inf')  # Maximum position limit
    backlash: int = 0  # Backlash compensation in steps
    max_speed: int = 10000  # Maximum speed in steps/s
    initial_speed: int = 10000  # Initial/default speed in steps/s
    acceleration: int = 1000000  # Acceleration in steps/s²

    def to_dict(self) -> dict:
        # Handle inf values for JSON serialization
        d = asdict(self)
        if d['min_pos'] == float('-inf'):
            d['min_pos'] = None
        if d['max_pos'] == float('inf'):
            d['max_pos'] = None
        return d

    @classmethod
    def from_dict(cls, data: dict) -> 'MotionSettings':
        if data is None:
            return cls()
        # Handle None values from JSON
        if data.get('min_pos') is None:
            data['min_pos'] = float('-inf')
        if data.get('max_pos') is None:
            data['max_pos'] = float('inf')
        return cls(**{k: v for k, v in data.items() if k in cls.__dataclass_fields__})


@dataclass
class SoftLimitSettings:
    """Soft limit configuration for a single axis."""
    enabled: bool = False
    min_pos: int = 0
    max_pos: int = 0

    def to_dict(self) -> dict:
        return asdict(self)

    @classmethod
    def from_dict(cls, data: dict) -> 'SoftLimitSettings':
        if data is None:
            return cls()
        return cls(**{k: v for k, v in data.items() if k in cls.__dataclass_fields__})


@dataclass
class HardLimitSettings:
    """Hard limit (emergency stop) configuration for a single axis."""
    enabled: bool = True  # Hard limits enabled by default
    polarity: int = 0  # 0=NO (Normally Open), 1=NC (Normally Closed)

    def to_dict(self) -> dict:
        return asdict(self)

    @classmethod
    def from_dict(cls, data: dict) -> 'HardLimitSettings':
        if data is None:
            return cls()
        return cls(**{k: v for k, v in data.items() if k in cls.__dataclass_fields__})


@dataclass
class AxisConfig:
    """Complete configuration for a single motor axis."""
    axis: str  # Axis name: "X", "Y", "Z", or "A"
    motion: MotionSettings = field(default_factory=MotionSettings)
    homing: HomingSettings = field(default_factory=HomingSettings)
    tmc: TMCSettings = field(default_factory=TMCSettings)
    soft_limits: SoftLimitSettings = field(default_factory=SoftLimitSettings)
    hard_limits: HardLimitSettings = field(default_factory=HardLimitSettings)
    joystick_inverted: bool = False

    def to_dict(self) -> dict:
        return {
            'axis': self.axis,
            'motion': self.motion.to_dict(),
            'homing': self.homing.to_dict(),
            'tmc': self.tmc.to_dict(),
            'soft_limits': self.soft_limits.to_dict(),
            'hard_limits': self.hard_limits.to_dict(),
            'joystick_inverted': self.joystick_inverted
        }

    @classmethod
    def from_dict(cls, data: dict) -> 'AxisConfig':
        if data is None:
            raise ValueError("Axis data cannot be None")
        return cls(
            axis=data.get('axis', 'X'),
            motion=MotionSettings.from_dict(data.get('motion')),
            homing=HomingSettings.from_dict(data.get('homing')),
            tmc=TMCSettings.from_dict(data.get('tmc')),
            soft_limits=SoftLimitSettings.from_dict(data.get('soft_limits')),
            hard_limits=HardLimitSettings.from_dict(data.get('hard_limits')),
            joystick_inverted=data.get('joystick_inverted', False)
        )


@dataclass
class MotorSystemConfig:
    """Complete motor system configuration for all axes."""
    axes: Dict[str, AxisConfig] = field(default_factory=dict)
    axis_order: List[int] = field(default_factory=lambda: [0, 1, 2, 3])  # A, X, Y, Z mapping
    is_corexy: bool = False
    is_enabled: bool = True
    enable_auto: bool = True
    is_dual_axis: bool = False  # Dual axis mode (A and Z linked)

    def __post_init__(self):
        # Initialize default axes if not provided
        if not self.axes:
            for axis_name in ['A', 'X', 'Y', 'Z']:
                self.axes[axis_name] = AxisConfig(axis=axis_name)

    def get_axis(self, axis: str) -> AxisConfig:
        """Get configuration for a specific axis."""
        axis = axis.upper()
        if axis not in self.axes:
            self.axes[axis] = AxisConfig(axis=axis)
        return self.axes[axis]

    def set_axis(self, axis: str, config: AxisConfig):
        """Set configuration for a specific axis."""
        axis = axis.upper()
        self.axes[axis] = config

    def to_dict(self) -> dict:
        return {
            'axes': {k: v.to_dict() for k, v in self.axes.items()},
            'axis_order': self.axis_order,
            'is_corexy': self.is_corexy,
            'is_enabled': self.is_enabled,
            'enable_auto': self.enable_auto,
            'is_dual_axis': self.is_dual_axis
        }

    @classmethod
    def from_dict(cls, data: dict) -> 'MotorSystemConfig':
        if data is None:
            return cls()
        axes = {}
        if 'axes' in data:
            for axis_name, axis_data in data['axes'].items():
                axes[axis_name] = AxisConfig.from_dict(axis_data)
        return cls(
            axes=axes,
            axis_order=data.get('axis_order', [0, 1, 2, 3]),
            is_corexy=data.get('is_corexy', False),
            is_enabled=data.get('is_enabled', True),
            enable_auto=data.get('enable_auto', True),
            is_dual_axis=data.get('is_dual_axis', False)
        )

    def to_json(self) -> str:
        """Serialize to JSON string."""
        return json.dumps(self.to_dict(), indent=2)

    @classmethod
    def from_json(cls, json_str: str) -> 'MotorSystemConfig':
        """Deserialize from JSON string."""
        return cls.from_dict(json.loads(json_str))

    @classmethod
    def from_imswitch_config(cls, manager_properties: dict, stage_offsets: dict = None) -> 'MotorSystemConfig':
        """
        Create MotorSystemConfig from ImSwitch positionerInfo.managerProperties format.
        
        This converts the existing ImSwitch configuration format to the unified format.
        """
        config = cls()
        
        # Global settings
        config.axis_order = manager_properties.get('axisOrder', [0, 1, 2, 3])
        config.is_corexy = manager_properties.get('isCoreXY', False)
        config.is_enabled = manager_properties.get('isEnable', True)
        config.enable_auto = manager_properties.get('enableauto', True)
        config.is_dual_axis = manager_properties.get('isDualaxis', False)
        
        # Per-axis settings
        for axis in ['X', 'Y', 'Z', 'A']:
            axis_config = config.get_axis(axis)
            
            # Motion settings
            axis_config.motion.step_size = manager_properties.get(f'stepsize{axis}', 1)
            axis_config.motion.min_pos = manager_properties.get(f'min{axis}', float('-inf'))
            axis_config.motion.max_pos = manager_properties.get(f'max{axis}', float('inf'))
            axis_config.motion.backlash = manager_properties.get(f'backlash{axis}', 0)
            axis_config.motion.max_speed = manager_properties.get(f'maxSpeed{axis}', 10000)
            axis_config.motion.initial_speed = manager_properties.get(f'initialSpeed{axis}', 10000)
            
            # Homing settings
            axis_config.homing.enabled = manager_properties.get(f'home{axis}enabled', False)
            axis_config.homing.speed = manager_properties.get(f'homeSpeed{axis}', 15000)
            axis_config.homing.direction = manager_properties.get(f'homeDirection{axis}', -1)
            axis_config.homing.endstop_polarity = manager_properties.get(f'homeEndstoppolarity{axis}', 1)
            axis_config.homing.endpos_release = manager_properties.get(f'homeEndposRelease{axis}', 3000)
            axis_config.homing.timeout = manager_properties.get(f'homeTimeout{axis}', 20000)
            axis_config.homing.home_on_start = manager_properties.get(f'homeOnStart{axis}', False)
            axis_config.homing.home_steps = manager_properties.get(f'homeSteps{axis}', 0)
            
            # TMC settings (if available)
            if manager_properties.get(f'msteps{axis}') is not None:
                axis_config.tmc.msteps = manager_properties.get(f'msteps{axis}', 16)
                axis_config.tmc.rms_current = manager_properties.get(f'rms_current{axis}', 500)
                axis_config.tmc.sgthrs = manager_properties.get(f'sgthrs{axis}', 10)
                axis_config.tmc.semin = manager_properties.get(f'semin{axis}', 5)
                axis_config.tmc.semax = manager_properties.get(f'semax{axis}', 2)
                axis_config.tmc.blank_time = manager_properties.get(f'blank_time{axis}', 24)
                axis_config.tmc.toff = manager_properties.get(f'toff{axis}', 3)
            
            config.set_axis(axis, axis_config)
        
        return config

    def to_imswitch_properties(self) -> dict:
        """
        Convert back to ImSwitch positionerInfo.managerProperties format.
        """
        props = {
            'axisOrder': self.axis_order,
            'isCoreXY': self.is_corexy,
            'isEnable': self.is_enabled,
            'enableauto': self.enable_auto,
            'isDualaxis': self.is_dual_axis,
        }
        
        for axis_name, axis_config in self.axes.items():
            # Motion settings
            props[f'stepsize{axis_name}'] = axis_config.motion.step_size
            if axis_config.motion.min_pos != float('-inf'):
                props[f'min{axis_name}'] = axis_config.motion.min_pos
            if axis_config.motion.max_pos != float('inf'):
                props[f'max{axis_name}'] = axis_config.motion.max_pos
            props[f'backlash{axis_name}'] = axis_config.motion.backlash
            props[f'maxSpeed{axis_name}'] = axis_config.motion.max_speed
            props[f'initialSpeed{axis_name}'] = axis_config.motion.initial_speed
            
            # Homing settings
            props[f'home{axis_name}enabled'] = axis_config.homing.enabled
            props[f'homeSpeed{axis_name}'] = axis_config.homing.speed
            props[f'homeDirection{axis_name}'] = axis_config.homing.direction
            props[f'homeEndstoppolarity{axis_name}'] = axis_config.homing.endstop_polarity
            props[f'homeEndposRelease{axis_name}'] = axis_config.homing.endpos_release
            props[f'homeTimeout{axis_name}'] = axis_config.homing.timeout
            props[f'homeOnStart{axis_name}'] = axis_config.homing.home_on_start
            props[f'homeSteps{axis_name}'] = axis_config.homing.home_steps
            
            # TMC settings
            props[f'msteps{axis_name}'] = axis_config.tmc.msteps
            props[f'rms_current{axis_name}'] = axis_config.tmc.rms_current
            props[f'sgthrs{axis_name}'] = axis_config.tmc.sgthrs
            props[f'semin{axis_name}'] = axis_config.tmc.semin
            props[f'semax{axis_name}'] = axis_config.tmc.semax
            props[f'blank_time{axis_name}'] = axis_config.tmc.blank_time
            props[f'toff{axis_name}'] = axis_config.tmc.toff
        
        return props
