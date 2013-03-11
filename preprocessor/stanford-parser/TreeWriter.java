/**
 * @file TreeWriter.java
 * Contains some simple preprocessing code in Java using the Stanford Parser.
 */

import edu.stanford.nlp.ling.HasWord;
import edu.stanford.nlp.parser.lexparser.LexicalizedParser;
import edu.stanford.nlp.process.DocumentPreprocessor;
import edu.stanford.nlp.trees.Tree;
import java.io.BufferedReader;
import java.io.FileWriter;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * Uses the Stanford Parser to preprocess text documents into parse trees and save them to disk.
 */
public class TreeWriter
{
    /**
     * Reads a file and returns all its lines.
     * @param filename - the file to read from
     * @return lines an ArrayList of lines from the file
     */
    private static ArrayList<String> readLines(String filename)
    {
        ArrayList<String> lines = new ArrayList<String>();
        try
        {
            String line = null;
            BufferedReader bufferedReader = new BufferedReader(new FileReader(filename));
            while((line = bufferedReader.readLine()) != null)
                lines.add(line);
            bufferedReader.close();
        }
        catch(Exception e)
        {
            System.out.println("Error reading " + filename);
        }
        return lines;
    }

    /**
     * Preprocesses the document via the Stanford Parser's DocumentPreprocessor, finding sentence boundaries.
     * @param filename - The document to process
     * @return a list of sentences
     */
    private static List<List<? extends HasWord>> setupSentences(String filename)
    {
        DocumentPreprocessor dp = new DocumentPreprocessor(filename);
        List<List<? extends HasWord>> ret = new ArrayList<List<? extends HasWord>>();
        for(List<HasWord> sentence : dp)
            ret.add(sentence);
        return ret;
    }
    
    /**
     * Runs the Stanford Parser on an array of filenames.
     * @param filenames - the files to run the parser on
     * @param maxSentenceLength - the maximum length of sentences that will be parsed
     */
    public static void process(ArrayList<String> filenames, String maxSentenceLength) throws IOException
    {
        LexicalizedParser parser = new LexicalizedParser("../../data/englishPCFG.ser.gz");
        parser.setOptionFlags("-maxLength", maxSentenceLength, "-retainTmpSubcategories");
        int filesDone = 0;
        for(String filename: filenames)
        {
            FileWriter writer = new FileWriter(filename + ".tree");
            System.out.println("Parsing " + filename + " ... ");
            String numDone = "[" + (filesDone + 1) + "/" + filenames.size() + "]: ";
            Iterable<List<? extends HasWord>> sentences = setupSentences(filename);
            double totalSentences = ((ArrayList<List<? extends HasWord>>)sentences).size();
            double sentencesProcessed = 0;
            for(List<? extends HasWord> sentence : sentences)
            {
                Tree parseTree = parser.apply(sentence);
                String parsed = treeToString(parseTree.firstChild());
                writer.write(parsed + "\n");
                System.out.print("  " + numDone);
                System.out.printf("%2.2f%% ", sentencesProcessed / totalSentences * 100.0);
                System.out.print(parsed.substring(0, Math.min(100, parsed.length())) + " ... \n");
                ++sentencesProcessed;
            }
            System.out.println();
            writer.close();
            ++filesDone;
        }
    }
    
    /**
     * Converts a Parse Tree into a String representation.
     * @param tree - the Parse Tree to convert
     * @return the string representation, using parens to distinguish levels
     */
    private static String treeToString(Tree tree)
    {
        if(tree.isLeaf())
            return "";
        
        String retString = "(" + tree.value();
        for(Tree node : tree.children())
            retString += treeToString(node);
        retString += ")";
        
        return retString;
    }
    
    /**
     * Simply calls the Stanford Parser on the command line arguments.
     */
    public static void main(String args[]) throws IOException
    {
        ArrayList<String> lines = readLines(args[0]);
        System.out.println("Processing " + lines.size() + " files.");
        process(lines, "100");
    }
}
