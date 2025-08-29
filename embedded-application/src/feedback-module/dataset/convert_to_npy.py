import sofa
import numpy as np

AZ_FOV = 56
EL_FOV = 42

hrir = sofa.Database.open(f'./hrir_dataset.sofa')

fs = hrir.Data.SamplingRate.get_values()[0] # type: ignore
positions = hrir.Source.Position.get_values(system='spherical') # type: ignore
dimensions = hrir.Dimensions.N # type: ignore

print('=' * 75)
print(f'Sampling rate: {fs} Hz')
print(f'Number of HRIRs: {len(positions)}')
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

    H = np.zeros((dimensions, 2), dtype=np.float64) # type: ignore
    H[:, 0] = hrir.Data.IR.get_values(indices={'M':idx, 'R':0, 'E':0}) # type: ignore
    H[:, 1] = hrir.Data.IR.get_values(indices={'M':idx, 'R':1, 'E':0}) # type: ignore

    output_positions.append(np.array([azimuth, elevation, distance], dtype=np.float64))
    output_hrirs.append(H)

output_positions = np.array(output_positions, dtype=np.float64)
output_hrirs = np.array(output_hrirs, dtype=np.float64)

print('=' * 75)
print(f'Output positions shape for dataset: {output_positions.shape}')
print(f'Output HRIRs shape for dataset: {output_hrirs.shape}')
print('=' * 75)

np.save(f'./positions.npy', output_positions)
np.save(f'./hrirs.npy', output_hrirs)
