#!/usr/bin/env python3
"""
Test Output Validator
Compares actual test output with expected behavior
"""

import re
import sys
from typing import Dict, List, Tuple

class TestValidator:
    def __init__(self):
        self.results = []
        
    def validate_memory_metrics(self, output: str, expected_total: int) -> bool:
        """Validate that used + free = total always holds"""
        pattern = r'used_memory = (\d+).*?free_memory = (\d+)'
        matches = re.findall(pattern, output, re.DOTALL)
        
        for used, free in matches:
            used_int = int(used)
            free_int = int(free)
            if used_int + free_int != expected_total:
                self.results.append(f"❌ Memory invariant violated: {used_int} + {free_int} != {expected_total}")
                return False
                
        self.results.append(f"✓ Memory invariant holds: used + free = {expected_total}")
        return True
        
    def validate_hit_ratio(self, output: str) -> bool:
        """Validate hit ratio is in valid range [0, 1]"""
        pattern = r'hit_ratio = ([\d.]+)'
        matches = re.findall(pattern, output)
        
        for ratio in matches:
            ratio_float = float(ratio)
            if ratio_float < 0.0 or ratio_float > 1.0:
                self.results.append(f"❌ Invalid hit ratio: {ratio_float}")
                return False
                
        self.results.append(f"✓ All hit ratios in valid range [0, 1]")
        return True
        
    def validate_offset_preservation(self, output: str) -> bool:
        """Validate that virtual and physical offsets match"""
        pattern = r'offset.*?0x([0-9a-fA-F]+).*?Physical offset.*?0x([0-9a-fA-F]+)'
        matches = re.findall(pattern, output, re.DOTALL | re.IGNORECASE)
        
        for virt_offset, phys_offset in matches:
            if virt_offset.lower() != phys_offset.lower():
                self.results.append(f"❌ Offset mismatch: virtual=0x{virt_offset} != physical=0x{phys_offset}")
                return False
                
        self.results.append(f"✓ All offsets preserved in address translation")
        return True
        
    def validate_power_of_two(self, output: str, field_name: str) -> bool:
        """Validate that specified field is always power of 2"""
        pattern = rf'{field_name}.*?(\d+)'
        matches = re.findall(pattern, output)
        
        for value in matches:
            val_int = int(value)
            if val_int > 0 and (val_int & (val_int - 1)) != 0:
                self.results.append(f"❌ Not power of 2: {field_name}={val_int}")
                return False
                
        self.results.append(f"✓ All {field_name} values are power of 2")
        return True
        
    def compare_expected_actual(self, output: str) -> Tuple[int, int]:
        """Count matches and mismatches between EXPECTED and ACTUAL"""
        pattern = r'\[EXPECTED\].*?=\s*(.+?)\\n.*?\[ACTUAL\].*?=\s*(.+?)\\n'
        matches = re.findall(pattern, output, re.DOTALL)
        
        match_count = 0
        mismatch_count = 0
        
        for expected, actual in matches:
            expected_clean = expected.strip()
            actual_clean = actual.strip()
            
            if expected_clean == actual_clean:
                match_count += 1
            else:
                mismatch_count += 1
                self.results.append(f"❌ Mismatch: expected '{expected_clean}' != actual '{actual_clean}'")
                
        return match_count, mismatch_count
        
    def validate_test_output(self, test_name: str, output: str) -> bool:
        """Main validation function"""
        print(f"\n{'='*60}")
        print(f"Validating: {test_name}")
        print(f"{'='*60}\n")
        
        all_passed = True
        
        # Count passed tests
        passed = output.count("PASSED")
        failed = output.count("FAILED")
        print(f"Tests PASSED: {passed}")
        print(f"Tests FAILED: {failed}")
        
        # Test-specific validations
        if "PhysicalMemory" in test_name:
            all_passed &= self.validate_memory_metrics(output, 1024)
            
        if "Cache" in test_name:
            all_passed &= self.validate_hit_ratio(output)
            
        if "VirtualMemory" in test_name:
            all_passed &= self.validate_offset_preservation(output)
            
        if "Buddy" in test_name:
            all_passed &= self.validate_power_of_two(output, "allocated_size")
            
        # Compare EXPECTED vs ACTUAL
        matches, mismatches = self.compare_expected_actual(output)
        print(f"\nExpected vs Actual:")
        print(f"  Matches: {matches}")
        print(f"  Mismatches: {mismatches}")
        
        if mismatches > 0:
            all_passed = False
            
        # Print detailed results
        print(f"\nValidation Results:")
        for result in self.results:
            print(f"  {result}")
            
        self.results.clear()
        
        return all_passed

def main():
    if len(sys.argv) < 2:
        print("Usage: python validate_test_output.py <test_output_file>")
        print("Example: python validate_test_output.py logs/test_physical_memory.log")
        sys.exit(1)
        
    validator = TestValidator()
    
    with open(sys.argv[1], 'r') as f:
        output = f.read()
        
    test_name = sys.argv[1].split('/')[-1].split('.')[0]
    success = validator.validate_test_output(test_name, output)
    
    if success:
        print("\n" + "="*60)
        print("✓ ALL VALIDATIONS PASSED")
        print("="*60)
        sys.exit(0)
    else:
        print("\n" + "="*60)
        print("✗ SOME VALIDATIONS FAILED")
        print("="*60)
        sys.exit(1)

if __name__ == "__main__":
    main()
