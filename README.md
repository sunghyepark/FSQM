# FSQM
: A Fast and Scalable Qubit-Mapping Method for NISQ (developed in C++)

## Contact
This code is written by [Sunghye Park](shpark96@postech.ac.kr) and [Minhyuk Kweon](mh.kweon@postech.ac.kr)

If you have any questions feel free to contact us using shpark96@postech.ac.kr

(CSDL Lab. in POSTECH, South Korea. http://csdl.postech.ac.kr)

## Related paper
- Title: [A Fast and Scalable Qubit-Mapping Method for Noisy Intermediate-Scale Quantum Computers](https://dl.acm.org/doi/abs/10.1145/3489517.3530402)
- Authors: S.Park, D.Kim, M.Kweon, J.-Y.Sim, and S.Kang
- Conference: ACM/IEEE Design Automation Conference (DAC), 2022

## Overview

## Usage
### Installation
```
[~/FSQM] mkdir build && cd build
[~/FSQM/build] cmake ..
[~/FSQM/build] make -j
```

### Run
```
[~/FSQM/tool] python3 run.py [circuit_index]
```
or
```
[~/FSQM/tool] python3 run.py all
```


If you want to know more about "run.py", put this command `python3 run.py help'

## Reference
If you use out mapping algorithm for your research, we would be thankful if you referred to it by citing the following publication
```
@inproceedings{park2022fast,
  title={A fast and scalable qubit-mapping method for noisy intermediate-scale quantum computers},
  author={Park, Sunghye and Kim, Daeyeon and Kweon, Minhyuk and Sim, Jae-Yoon and Kang, Seokhyeong},
  booktitle={Proceedings of the 59th ACM/IEEE Design Automation Conference},
  pages={13--18},
  year={2022}
}
```

