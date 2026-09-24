// ExportDecomp.java - Ghidra headless: decompile every function to <outDir>
// usage: analyzeHeadless <proj> <name> -import <bin> -scriptPath <dir> \
//            -postScript ExportDecomp.java <outDir>
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import java.io.*;

public class ExportDecomp extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        String outDir = args.length > 0 ? args[0] : "/tmp/ghidra_decomp";
        File d = new File(outDir);
        d.mkdirs();

        DecompInterface di = new DecompInterface();
        di.toggleCCode(true);
        di.toggleSyntaxTree(true);
        di.openProgram(currentProgram);

        FunctionManager fm = currentProgram.getFunctionManager();
        int n = 0, ok = 0;
        try (FileWriter index = new FileWriter(new File(d, "_index.txt"))) {
            for (Function f : fm.getFunctions(true)) {
                n++;
                DecompileResults r = null;
                try {
                    r = di.decompileFunction(f, 30, monitor);
                } catch (Exception e) { /* skip */ }
                if (r != null && r.decompileCompleted()) {
                    ok++;
                    String c = r.getDecompiledFunction().getC();
                    String nm = f.getName().replaceAll("[^A-Za-z0-9_.-]", "_");
                    File out = new File(d, String.format("%08x_%s.c",
                            f.getEntryPoint().getOffset(), nm));
                    try (FileWriter fw = new FileWriter(out)) { fw.write(c); }
                    index.write(String.format("%08x %s %s%n",
                            f.getEntryPoint().getOffset(), f.getName(), out.getName()));
                }
            }
        }
        println("ExportDecomp: " + ok + "/" + n + " functions -> " + outDir);
    }
}
