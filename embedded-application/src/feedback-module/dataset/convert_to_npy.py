import sofa
import numpy as np

AZ_FOV = 62.8
EL_FOV = 40

dataset = sofa.Database.open(f'./hrir_dataset.sofa')

fs = dataset.Data.SamplingRate.get_values()[0] # type: ignore
hrirs = dataset.Data.IR.get_values() # type: ignore
positions = dataset.Source.Position.get_values(system='spherical') # type: ignore
dimensions = dataset.Dimensions.N # type: ignore

print('=' * 75)
print(f'Sampling rate: {fs} Hz')
print(f'Number of HRIRs: {len(hrirs)}')
print(f'Number of positions: {len(positions)}')
print(f'HRIR dimensions: {dimensions}')
print('=' * 75)

output_positions = []
output_hrirs = []

for idx in range(len(positions)):
    position = positions[idx]

    azimuth = position[0]
    elevation = position[1]
    distance = position[2]

    if ((azimuth > (AZ_FOV / 2 + 10)) and (azimuth < (360 - AZ_FOV / 2 - 10))):
        continue
    if ((elevation > (EL_FOV / 2 + 10)) or (elevation < -(EL_FOV / 2 + 10))):
        continue
    
    print(f'Processing HRIR {idx + 1}/{len(positions)}')

    H = np.zeros((dimensions, 2), dtype=np.float32) # type: ignore
    H[:, 0] = hrirs[idx, 0, :]
    H[:, 1] = hrirs[idx, 1, :]

    output_positions.append(np.array([azimuth, elevation, distance], dtype=np.float32))
    output_hrirs.append(H)

output_positions = np.array(output_positions, dtype=np.float32)
output_hrirs = np.array(output_hrirs, dtype=np.float32)

print('=' * 75)
print(f'Output positions shape for dataset: {output_positions.shape}')
print(f'Output HRIRs shape for dataset: {output_hrirs.shape}')
print('=' * 75)

np.save(f'./positions.npy', output_positions)
np.save(f'./hrirs.npy', output_hrirs)
