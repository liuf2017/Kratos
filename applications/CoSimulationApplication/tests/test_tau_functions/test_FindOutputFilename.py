import sys, os, unittest
import numpy as np

tau_functions_path = os.path.join(os.path.dirname(__file__), '../../python_scripts/helpers')
sys.path.append(tau_functions_path)

import tau_functions as TauFunctions


class TestFindOutputFilename(unittest.TestCase):

    def test_FindOutputFilename(self):
        # Create dummy file
        os.mkdir('Outputs')
        path = os.getcwd() + '/'
        pattern = 'airfoilSol.pval.unsteady_i='
        step = 304
        reference_file_name = path + 'Outputs/' + pattern + str(step + 1)
        open(reference_file_name, 'w').close()

        # Retrive the file
        file_name = TauFunctions.FindOutputFilename(path, step)

        # Check
        self.assertMultiLineEqual(file_name, reference_file_name)

        # Remove dummy file and directory
        os.remove(file_name)
        os.rmdir('Outputs')


if __name__ == '__main__':
    unittest.main()

