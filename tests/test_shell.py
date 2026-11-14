import subprocess
import unittest
import tempfile
import os
import ctypes

class TestShell(unittest.TestCase):
    """Base test case class with helper methods to run and verify shell output."""


    SHELL_BINARY = './gauri-shell'

    
    def run_shell(self, commands_input, timeout=3):
        """
        Helper method: Spawns the shell binary, feeds commands into stdin,
        and returns stdout, stderr, and the process exit code.
        """
        try:
            process = subprocess.run(
                [self.SHELL_BINARY],
                input=commands_input,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                timeout=timeout
            )
            return process.stdout, process.stderr, process.returncode
        except subprocess.TimeoutExpired:
            self.fail(f"Shell hung (timed out) on input:\n{commands_input}")


class TestBasicCommands(TestShell):
    """Test Suite 1: Valid basic commands and built-ins."""

    def test_basic_command(self):
        # echo
        stdout, stderr, code = self.run_shell("echo 'hello systems'\nexit\n")
        self.assertEqual(code, 0)
        self.assertIn("hello systems", stdout)


    def test_builtin_cd(self):
        stdout, stderr, code = self.run_shell("cd /\npwd\nexit\n")
        self.assertEqual(code, 0)
        self.assertIn("/", stdout)

    def test_builtin_help(self):
        stdout, stderr, code = self.run_shell("help\nexit\n")
        self.assertEqual(code, 0)
        self.assertIn("shell commands:", stdout)
        self.assertIn("cd: change directory to path specified as argument.", stdout)
        self.assertIn("source: execute script given filename as argument and each line of file as command.", stdout)
        self.assertIn("prev: prints previous command line and executes it again without becoming the new command line.", stdout)
        self.assertIn("help: explains all commands available in shell.", stdout)

    def test_builtin_prev(self):
        stdout, stderr, code = self.run_shell("echo 1\nprev\nexit\n")
        self.assertEqual(code, 0)
        self.assertIn("1\n1", stdout)

    def test_builtin_source(self):
        with tempfile.NamedTemporaryFile(mode='w', delete=False, suffix='.sh') as f:
            f.write("echo 'script line one'\n")
            f.write("echo 'script line two'\n")
            script_path = f.name

        try:
            stdout, stderr, code = self.run_shell(f"source {script_path}\nexit\n")
            self.assertEqual(code, 0)
            self.assertIn("script line one\nscript line two", stdout)
        finally:
            if os.path.exists(script_path):
                os.remove(script_path)

class TestInvalidBasicCommands(TestShell):
    """Test Suite 2: Invalid basic commands and built-ins."""

    def test_invalid_basic_command(self):
        stdout, stderr, code = self.run_shell("randomcommand\nexit\n");
        self.assertEqual(code, 0);
        self.assertIn("randomcommand: command not found.", stderr);

    def test_invalid_builtin_cd(self):
        stdout, stderr, code = self.run_shell("cd fjdafsfdslf\nexit\n");
        self.assertEqual(code, 0);
        self.assertIn("cd command: system cannot find path specified.", stderr);

    def test_invalid_builtin_source(self):
        # invalid source file
        stdout, stderr, code = self.run_shell("source jfjdaskfjadsklfjs\nexit\n");
        self.assertEqual(code, 0);
        self.assertIn("source command: file open failed.", stderr);
        
        # no source file
        stdout, stderr, code = self.run_shell("source\nexit\n");
        self.assertEqual(code, 0);
        self.assertIn("source command: no file provided.", stderr);


if __name__ == '__main__':
    unittest.main()
